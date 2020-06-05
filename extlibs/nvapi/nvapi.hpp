#ifdef _WIN64
	const char *NVAPI_DLL = "nvapi64.dll";
#else
	const char *NVAPI_DLL = "nvapi.dll";
#endif

#include "nvapi_definitions.hpp"

namespace {

	std::string nvapi_get_error_msg( NvAPI_Status status )
	{
		NvAPI_ShortString msg = {0};
		NvAPI_GetErrorMessage( status, msg );
		char c_msg[255];
		sprintf( c_msg, "%s \n\0", msg );
		return c_msg;
	}

	void nvapi_set_ustring( NvAPI_UnicodeString& nvStr, const wchar_t* wcStr )
	{
		for ( int i = 0; i < NVAPI_UNICODE_STRING_MAX; i++ )
			nvStr[i] = 0;

		int i = 0;
		while ( wcStr[i] != 0 )
		{
			nvStr[i] = wcStr[i];
			i++;
		}
	}

	int nvapi_init()
	{
		const bool disableThreadedOptimizations   = true;
		const bool setMaximumPerformance          = true;
		const bool disableTripleBuffering         = true;
		const bool setPrerenderedFrames           = true;

		const wchar_t* profileName                = L"Attract Mode";
		const wchar_t* appName                    = L"";

		int return_code = 0;

		std::wstring file_name;
		wchar_t result[MAX_PATH];
		file_name = std::wstring( result, GetModuleFileNameW( NULL, result, MAX_PATH ));
		size_t found = file_name.find_last_of( L"/\\" );
		file_name = file_name.substr( found + 1 );
		appName = file_name.c_str();

		HMODULE hmod = LoadLibrary( NVAPI_DLL );
		if ( hmod == NULL )
		{
			FeDebug() << "NvAPI: " << NVAPI_DLL << " not found" << std::endl;
			return 0;
		}
		else
			FeDebug() << "NvAPI: " << NVAPI_DLL << " loaded" << std::endl;

		// nvapi_QueryInterface is a function used to retrieve other internal functions in nvapi.dll
		NvAPI_QueryInterface = (NvAPI_QueryInterface_t) GetProcAddress(hmod, "nvapi_QueryInterface");

		// entry points to nvapi.dll internal functions
		NvAPI_Initialize = (NvAPI_Initialize_t) (*NvAPI_QueryInterface)(0x0150E828);
		NvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUs_t) (*NvAPI_QueryInterface)(0xE5AC921F);
		NvAPI_GPU_GetUsages = (NvAPI_GPU_GetUsages_t) (*NvAPI_QueryInterface)(0x189A1FDF);
		NvAPI_DRS_CreateProfile = (NvAPI_DRS_CreateProfile_t) (*NvAPI_QueryInterface)(0xCC176068);
		NvAPI_DRS_DeleteProfile = (NvAPI_DRS_DeleteProfile_t) (*NvAPI_QueryInterface)(0x17093206);
		NvAPI_DRS_CreateSession = (NvAPI_DRS_CreateSession_t) (*NvAPI_QueryInterface)(0x0694D52E);
		NvAPI_DRS_LoadSettings = (NvAPI_DRS_LoadSettings_t) (*NvAPI_QueryInterface)(0x375DBD6B);
		NvAPI_DRS_CreateApplication = (NvAPI_DRS_CreateApplication_t) (*NvAPI_QueryInterface)(0x4347A9DE);
		NvAPI_DRS_SetSetting = (NvAPI_DRS_SetSetting_t) (*NvAPI_QueryInterface)(0x577DD202);
		NvAPI_DRS_SaveSettings = (NvAPI_DRS_SaveSettings_t) (*NvAPI_QueryInterface)(0xFCBC7E14);
		NvAPI_DRS_DestroySession = (NvAPI_DRS_DestroySession_t) (*NvAPI_QueryInterface)(0x0DAD9CFF8);
		NvAPI_DRS_FindProfileByName = (NvAPI_DRS_FindProfileByName_t) (*NvAPI_QueryInterface)(0x7E4A9A0B);
		NvAPI_DRS_FindApplicationByName	= (NvAPI_DRS_FindApplicationByName_t) (*NvAPI_QueryInterface)(0xEEE566B2);
		NvAPI_DRS_GetProfileInfo = (NvAPI_DRS_GetProfileInfo_t) (*NvAPI_QueryInterface)(0x61CD6FD6);
		NvAPI_GetErrorMessage = (NvAPI_GetErrorMessage_t) (*NvAPI_QueryInterface)(0x6C2D048C);

		NvDRSSessionHandle hSession;
		NvAPI_Status status;

		// Initialize NvAPI
		status = NvAPI_Initialize();
		FeDebug() << "NvAPI: Initialize: " << nvapi_get_error_msg( status );
		if ( status != NVAPI_OK )
			return 0;

		status = NvAPI_DRS_CreateSession( &hSession );
		FeDebug() << "NvAPI: CreateSession: " << nvapi_get_error_msg( status );

		status = NvAPI_DRS_LoadSettings( hSession );
		FeDebug() << "NvAPI: LoadSettings: " << nvapi_get_error_msg( status );

		// Fill application info
		NVDRS_APPLICATION app;
		app.version                     = NVDRS_APPLICATION_VER;
		app.isPredefined                = 0;
		nvapi_set_ustring( app.appName, appName );
		nvapi_set_ustring( app.launcher, L"" );

		NvDRSProfileHandle hProfile;

		status = NvAPI_DRS_FindApplicationByName(hSession, app.appName, &hProfile, &app);
		if ( status != NVAPI_OK )
			return_code = 1;

		NVDRS_PROFILE profileInfo;
		profileInfo.version             = NVDRS_PROFILE_VER;
		profileInfo.isPredefined        = 0;

    	NvAPI_DRS_GetProfileInfo(hSession, hProfile, &profileInfo);

		if ( status == NVAPI_OK )
		{
			if ( std::wcscmp((wchar_t*)profileInfo.profileName, profileName) != 0 )
			{
				FeDebug() << "NvAPI: " << nowide::narrow( file_name ) << " is already assigned in profile " <<
					nowide::narrow((wchar_t*)profileInfo.profileName) << std::endl;
				status = NvAPI_DRS_DeleteProfile( hSession, hProfile );
				FeDebug() << "NvAPI: Deleting Nvidia profile: " << nvapi_get_error_msg( status );
				return_code = 1;
			}
		}

		// Fill profile info
		nvapi_set_ustring( profileInfo.profileName, profileName );

		// Open profile or create if not found
		status = NvAPI_DRS_FindProfileByName( hSession, profileInfo.profileName, &hProfile );
		FeDebug() << "NvAPI: FindProfileByName: " << nvapi_get_error_msg( status );

		if ( status == NVAPI_PROFILE_NOT_FOUND )
		{
			status = NvAPI_DRS_CreateProfile( hSession, &profileInfo, &hProfile );
			FeDebug() << "NvAPI: CreateProfile: " << nvapi_get_error_msg( status );
		}

		// Create application
		status = NvAPI_DRS_CreateApplication( hSession, hProfile, &app );
		if ( status != NVAPI_ERROR )
			FeDebug() << "NvAPI: CreateProfile: " << nvapi_get_error_msg( status );

		if ( status == NVAPI_EXECUTABLE_ALREADY_IN_USE )
		{
			FeLog() << "NvAPI ERROR: " << nowide::narrow( file_name ) << " is already assigned in another nvidia profile" << std::endl;
			FeLog() << std::endl;
			status = NvAPI_DRS_DestroySession( hSession );
			FeDebug() << "NvAPI: Closing Nvidia session: " << nvapi_get_error_msg( status );
			return 0;
		}

		NVDRS_SETTING setting;

		// Set Threaded Optimizations
		if ( disableThreadedOptimizations )
		{
			setting.version                 = NVDRS_SETTING_VER;
			setting.settingId               = OGL_THREAD_CONTROL_ID;
			setting.settingType             = NVDRS_DWORD_TYPE;
			setting.settingLocation         = NVDRS_CURRENT_PROFILE_LOCATION;
			setting.isCurrentPredefined     = 0;
			setting.isPredefinedValid       = 0;
			setting.u32CurrentValue         = OGL_THREAD_CONTROL_DISABLE;
			setting.u32PredefinedValue      = OGL_THREAD_CONTROL_DISABLE;

			status = NvAPI_DRS_SetSetting( hSession, hProfile, &setting );
			FeDebug() << "NvAPI: Threaded Optimizations OFF: " << nvapi_get_error_msg( status );
		}

		// Set Triple Buffering
		if ( disableTripleBuffering )
		{
			setting.version                 = NVDRS_SETTING_VER;
			setting.settingId               = OGL_TRIPLE_BUFFER_ID;
			setting.settingType             = NVDRS_DWORD_TYPE;
			setting.settingLocation         = NVDRS_CURRENT_PROFILE_LOCATION;
			setting.isCurrentPredefined     = 0;
			setting.isPredefinedValid       = 0;
			setting.u32CurrentValue         = OGL_TRIPLE_BUFFER_DISABLED;
			setting.u32PredefinedValue      = OGL_TRIPLE_BUFFER_DISABLED;

			status = NvAPI_DRS_SetSetting( hSession, hProfile, &setting );
			FeDebug() << "NvAPI: Triple Buffering OFF: " << nvapi_get_error_msg( status );
		}

		// Set Prerendered Frames
		if ( setPrerenderedFrames )
		{
			setting.version                 = NVDRS_SETTING_VER;
			setting.settingId               = PRERENDERLIMIT_ID;
			setting.settingType             = NVDRS_DWORD_TYPE;
			setting.settingLocation         = NVDRS_CURRENT_PROFILE_LOCATION;
			setting.isCurrentPredefined     = 0;
			setting.isPredefinedValid       = 0;
			setting.u32CurrentValue         = 0x04;
			setting.u32PredefinedValue      = 0x04;

			status = NvAPI_DRS_SetSetting( hSession, hProfile, &setting );
			FeDebug() << "NvAPI: Prendered Frames set to 4: " << nvapi_get_error_msg( status );

			setting.version                 = NVDRS_SETTING_VER;
			setting.settingId               = OGL_MAX_FRAMES_ALLOWED_ID;
			setting.settingType             = NVDRS_DWORD_TYPE;
			setting.settingLocation         = NVDRS_CURRENT_PROFILE_LOCATION;
			setting.isCurrentPredefined     = 0;
			setting.isPredefinedValid       = 0;
			setting.u32CurrentValue         = 0x04;
			setting.u32PredefinedValue      = 0x04;

			status = NvAPI_DRS_SetSetting( hSession, hProfile, &setting );
			FeDebug() << "NvAPI: Maximum frames allowed set to 4: " << nvapi_get_error_msg( status );
		}

		// Set Power State
		if ( setMaximumPerformance )
		{
			setting.version                 = NVDRS_SETTING_VER;
			setting.settingId               = PREFERRED_PSTATE_ID;
			setting.settingType             = NVDRS_DWORD_TYPE;
			setting.settingLocation         = NVDRS_CURRENT_PROFILE_LOCATION;
			setting.isCurrentPredefined     = 0;
			setting.isPredefinedValid       = 0;
			setting.u32CurrentValue         = PREFERRED_PSTATE_PREFER_MAX;
			setting.u32PredefinedValue      = PREFERRED_PSTATE_PREFER_MAX;

			status = NvAPI_DRS_SetSetting( hSession, hProfile, &setting );
			FeDebug() << "NvAPI: GPU Maximum Performance ON: " << nvapi_get_error_msg( status );
		}

		// Save changes
		status = NvAPI_DRS_SaveSettings( hSession );
		FeDebug() << "NvAPI: Saving Nvidia profile: " << nvapi_get_error_msg( status );

		// Destroy session
		status = NvAPI_DRS_DestroySession( hSession );
		FeDebug() << "NvAPI: Closing Nvidia session: " << nvapi_get_error_msg( status );

		return return_code;
	}
}
