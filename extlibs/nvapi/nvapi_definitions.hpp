namespace {

#define NVAPI_MAX_PHYSICAL_GPUS   64
#define NVAPI_MAX_USAGES_PER_GPU  34
#define NVAPI_UNICODE_STRING_MAX  2048
#define NVAPI_BINARY_DATA_MAX     4096
#define NVAPI_SHORT_STRING_MAX    64

enum Esetting {
	OGL_THREAD_CONTROL_ID                               = 0x20C1221E,
	OGL_TRIPLE_BUFFER_ID                                = 0x20FDD1F9,
	PREFERRED_PSTATE_ID                                 = 0x1057EB71,
	PRERENDERLIMIT_ID                                   = 0x007BA09E,
	OGL_MAX_FRAMES_ALLOWED_ID                           = 0x208E55E3
};

enum EValues_OGL_THREAD_CONTROL {
	OGL_THREAD_CONTROL_ENABLE                           = 0x00000001,
	OGL_THREAD_CONTROL_DISABLE                          = 0x00000002,
	OGL_THREAD_CONTROL_NUM_VALUES                       = 2,
	OGL_THREAD_CONTROL_DEFAULT                          = 0
};

enum EValues_OGL_TRIPLE_BUFFER {
	OGL_TRIPLE_BUFFER_DISABLED                          = 0x00000000,
	OGL_TRIPLE_BUFFER_ENABLED                           = 0x00000001,
	OGL_TRIPLE_BUFFER_NUM_VALUES                        = 2,
	OGL_TRIPLE_BUFFER_DEFAULT = OGL_TRIPLE_BUFFER_DISABLED
};

enum EValues_PREFERRED_PSTATE {
	PREFERRED_PSTATE_ADAPTIVE                            = 0x00000000,
	PREFERRED_PSTATE_PREFER_MAX                          = 0x00000001,
	PREFERRED_PSTATE_DRIVER_CONTROLLED                   = 0x00000002,
	PREFERRED_PSTATE_PREFER_CONSISTENT_PERFORMANCE       = 0x00000003,
	PREFERRED_PSTATE_PREFER_MIN                          = 0x00000004,
	PREFERRED_PSTATE_OPTIMAL_POWER                       = 0x00000005,
	PREFERRED_PSTATE_MIN                                 = 0x00000000,
	PREFERRED_PSTATE_MAX                                 = 0x00000005,
	PREFERRED_PSTATE_NUM_VALUES = 8,
	PREFERRED_PSTATE_DEFAULT = PREFERRED_PSTATE_OPTIMAL_POWER
};

enum EValues_PRERENDERLIMIT {
    PRERENDERLIMIT_MIN                                   = 0x00,
    PRERENDERLIMIT_MAX                                   = 0xff,
    PRERENDERLIMIT_APP_CONTROLLED                        = 0x00,
    PRERENDERLIMIT_NUM_VALUES = 3,
    PRERENDERLIMIT_DEFAULT = PRERENDERLIMIT_APP_CONTROLLED
};

typedef enum _NvAPI_Status
{
	NVAPI_OK                                    =  0,      //!< Success. Request is completed.
	NVAPI_ERROR                                 = -1,      //!< Generic error
	NVAPI_LIBRARY_NOT_FOUND                     = -2,      //!< NVAPI support library cannot be loaded.
	NVAPI_NO_IMPLEMENTATION                     = -3,      //!< not implemented in current driver installation
	NVAPI_API_NOT_INITIALIZED                   = -4,      //!< NvAPI_Initialize has not been called (successfully)
	NVAPI_INVALID_ARGUMENT                      = -5,      //!< The argument/parameter value is not valid or NULL.
	NVAPI_NVIDIA_DEVICE_NOT_FOUND               = -6,      //!< No NVIDIA display driver, or NVIDIA GPU driving a display, was found.
	NVAPI_END_ENUMERATION                       = -7,      //!< No more items to enumerate
	NVAPI_INVALID_HANDLE                        = -8,      //!< Invalid handle
	NVAPI_INCOMPATIBLE_STRUCT_VERSION           = -9,      //!< An argument's structure version is not supported
	NVAPI_HANDLE_INVALIDATED                    = -10,     //!< The handle is no longer valid (likely due to GPU or display re-configuration)
	NVAPI_OPENGL_CONTEXT_NOT_CURRENT            = -11,     //!< No NVIDIA OpenGL context is current (but needs to be)
	NVAPI_INVALID_POINTER                       = -14,     //!< An invalid pointer, usually NULL, was passed as a parameter
	NVAPI_NO_GL_EXPERT                          = -12,     //!< OpenGL Expert is not supported by the current drivers
	NVAPI_INSTRUMENTATION_DISABLED              = -13,     //!< OpenGL Expert is supported, but driver instrumentation is currently disabled
	NVAPI_NO_GL_NSIGHT                          = -15,     //!< OpenGL does not support Nsight

	NVAPI_EXPECTED_LOGICAL_GPU_HANDLE           = -100,    //!< Expected a logical GPU handle for one or more parameters
	NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE          = -101,    //!< Expected a physical GPU handle for one or more parameters
	NVAPI_EXPECTED_DISPLAY_HANDLE               = -102,    //!< Expected an NV display handle for one or more parameters
	NVAPI_INVALID_COMBINATION                   = -103,    //!< The combination of parameters is not valid.
	NVAPI_NOT_SUPPORTED                         = -104,    //!< Requested feature is not supported in the selected GPU
	NVAPI_PORTID_NOT_FOUND                      = -105,    //!< No port ID was found for the I2C transaction
	NVAPI_EXPECTED_UNATTACHED_DISPLAY_HANDLE    = -106,    //!< Expected an unattached display handle as one of the input parameters.
	NVAPI_INVALID_PERF_LEVEL                    = -107,    //!< Invalid perf level
	NVAPI_DEVICE_BUSY                           = -108,    //!< Device is busy; request not fulfilled
	NVAPI_NV_PERSIST_FILE_NOT_FOUND             = -109,    //!< NV persist file is not found
	NVAPI_PERSIST_DATA_NOT_FOUND                = -110,    //!< NV persist data is not found
	NVAPI_EXPECTED_TV_DISPLAY                   = -111,    //!< Expected a TV output display
	NVAPI_EXPECTED_TV_DISPLAY_ON_DCONNECTOR     = -112,    //!< Expected a TV output on the D Connector - HDTV_EIAJ4120.
	NVAPI_NO_ACTIVE_SLI_TOPOLOGY                = -113,    //!< SLI is not active on this device.
	NVAPI_SLI_RENDERING_MODE_NOTALLOWED         = -114,    //!< Setup of SLI rendering mode is not possible right now.
	NVAPI_EXPECTED_DIGITAL_FLAT_PANEL           = -115,    //!< Expected a digital flat panel.
	NVAPI_ARGUMENT_EXCEED_MAX_SIZE              = -116,    //!< Argument exceeds the expected size.
	NVAPI_DEVICE_SWITCHING_NOT_ALLOWED          = -117,    //!< Inhibit is ON due to one of the flags in NV_GPU_DISPLAY_CHANGE_INHIBIT or SLI active.
	NVAPI_TESTING_CLOCKS_NOT_SUPPORTED          = -118,    //!< Testing of clocks is not supported.
	NVAPI_UNKNOWN_UNDERSCAN_CONFIG              = -119,    //!< The specified underscan config is from an unknown source (e.g. INF)
	NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO        = -120,    //!< Timeout while reconfiguring GPUs
	NVAPI_DATA_NOT_FOUND                        = -121,    //!< Requested data was not found
	NVAPI_EXPECTED_ANALOG_DISPLAY               = -122,    //!< Expected an analog display
	NVAPI_NO_VIDLINK                            = -123,    //!< No SLI video bridge is present
	NVAPI_REQUIRES_REBOOT                       = -124,    //!< NVAPI requires a reboot for the settings to take effect
	NVAPI_INVALID_HYBRID_MODE                   = -125,    //!< The function is not supported with the current Hybrid mode.
	NVAPI_MIXED_TARGET_TYPES                    = -126,    //!< The target types are not all the same
	NVAPI_SYSWOW64_NOT_SUPPORTED                = -127,    //!< The function is not supported from 32-bit on a 64-bit system.
	NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,    //!< There is no implicit GPU topology active. Use NVAPI_SetHybridMode to change topology.
	NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,      //!< Prompt the user to close all non-migratable applications.
	NVAPI_OUT_OF_MEMORY                         = -130,    //!< Could not allocate sufficient memory to complete the call.
	NVAPI_WAS_STILL_DRAWING                     = -131,    //!< The previous operation that is transferring information to or from this surface is incomplete.
	NVAPI_FILE_NOT_FOUND                        = -132,    //!< The file was not found.
	NVAPI_TOO_MANY_UNIQUE_STATE_OBJECTS         = -133,    //!< There are too many unique instances of a particular type of state object.
	NVAPI_INVALID_CALL                          = -134,    //!< The method call is invalid. For example, a method's parameter may not be a valid pointer.
	NVAPI_D3D10_1_LIBRARY_NOT_FOUND             = -135,    //!< d3d10_1.dll cannot be loaded.
	NVAPI_FUNCTION_NOT_FOUND                    = -136,    //!< Couldn't find the function in the loaded DLL.
	NVAPI_INVALID_USER_PRIVILEGE                = -137,    //!< Current User is not Admin.
	NVAPI_EXPECTED_NON_PRIMARY_DISPLAY_HANDLE   = -138,    //!< The handle corresponds to GDIPrimary.
	NVAPI_EXPECTED_COMPUTE_GPU_HANDLE           = -139,    //!< Setting Physx GPU requires that the GPU is compute-capable.
	NVAPI_STEREO_NOT_INITIALIZED                = -140,    //!< The Stereo part of NVAPI failed to initialize completely. Check if the stereo driver is installed.
	NVAPI_STEREO_REGISTRY_ACCESS_FAILED         = -141,    //!< Access to stereo-related registry keys or values has failed.
	NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED = -142, //!< The given registry profile type is not supported.
	NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED   = -143,    //!< The given registry value is not supported.
	NVAPI_STEREO_NOT_ENABLED                    = -144,    //!< Stereo is not enabled and the function needed it to execute completely.
	NVAPI_STEREO_NOT_TURNED_ON                  = -145,    //!< Stereo is not turned on and the function needed it to execute completely.
	NVAPI_STEREO_INVALID_DEVICE_INTERFACE       = -146,    //!< Invalid device interface.
	NVAPI_STEREO_PARAMETER_OUT_OF_RANGE         = -147,    //!< Separation percentage or JPEG image capture quality is out of [0-100] range.
	NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED = -148, //!< The given frustum adjust mode is not supported.
	NVAPI_TOPO_NOT_POSSIBLE                     = -149,    //!< The mosaic topology is not possible given the current state of the hardware.
	NVAPI_MODE_CHANGE_FAILED                    = -150,    //!< An attempt to do a display resolution mode change has failed.
	NVAPI_D3D11_LIBRARY_NOT_FOUND               = -151,    //!< d3d11.dll/d3d11_beta.dll cannot be loaded.
	NVAPI_INVALID_ADDRESS                       = -152,    //!< Address is outside of valid range.
	NVAPI_STRING_TOO_SMALL                      = -153,    //!< The pre-allocated string is too small to hold the result.
	NVAPI_MATCHING_DEVICE_NOT_FOUND             = -154,    //!< The input does not match any of the available devices.
	NVAPI_DRIVER_RUNNING                        = -155,    //!< Driver is running.
	NVAPI_DRIVER_NOTRUNNING                     = -156,    //!< Driver is not running.
	NVAPI_ERROR_DRIVER_RELOAD_REQUIRED          = -157,    //!< A driver reload is required to apply these settings.
	NVAPI_SET_NOT_ALLOWED                       = -158,    //!< Intended setting is not allowed.
	NVAPI_ADVANCED_DISPLAY_TOPOLOGY_REQUIRED    = -159,    //!< Information can't be returned due to "advanced display topology".
	NVAPI_SETTING_NOT_FOUND                     = -160,    //!< Setting is not found.
	NVAPI_SETTING_SIZE_TOO_LARGE                = -161,    //!< Setting size is too large.
	NVAPI_TOO_MANY_SETTINGS_IN_PROFILE          = -162,    //!< There are too many settings for a profile.
	NVAPI_PROFILE_NOT_FOUND                     = -163,    //!< Profile is not found.
	NVAPI_PROFILE_NAME_IN_USE                   = -164,    //!< Profile name is duplicated.
	NVAPI_PROFILE_NAME_EMPTY                    = -165,    //!< Profile name is empty.
	NVAPI_EXECUTABLE_NOT_FOUND                  = -166,    //!< Application not found in the Profile.
	NVAPI_EXECUTABLE_ALREADY_IN_USE             = -167,    //!< Application already exists in the other profile.
	NVAPI_DATATYPE_MISMATCH                     = -168,    //!< Data Type mismatch
	NVAPI_PROFILE_REMOVED                       = -169,    //!< The profile passed as parameter has been removed and is no longer valid.
	NVAPI_UNREGISTERED_RESOURCE                 = -170,    //!< An unregistered resource was passed as a parameter.
	NVAPI_ID_OUT_OF_RANGE                       = -171,    //!< The DisplayId corresponds to a display which is not within the normal outputId range.
	NVAPI_DISPLAYCONFIG_VALIDATION_FAILED       = -172,    //!< Display topology is not valid so the driver cannot do a mode set on this configuration.
	NVAPI_DPMST_CHANGED                         = -173,    //!< Display Port Multi-Stream topology has been changed.
	NVAPI_INSUFFICIENT_BUFFER                   = -174,    //!< Input buffer is insufficient to hold the contents.
	NVAPI_ACCESS_DENIED                         = -175,    //!< No access to the caller.
	NVAPI_MOSAIC_NOT_ACTIVE                     = -176,    //!< The requested action cannot be performed without Mosaic being enabled.
	NVAPI_SHARE_RESOURCE_RELOCATED              = -177,    //!< The surface is relocated away from video memory.
	NVAPI_REQUEST_USER_TO_DISABLE_DWM           = -178,    //!< The user should disable DWM before calling NvAPI.
	NVAPI_D3D_DEVICE_LOST                       = -179,    //!< D3D device status is D3DERR_DEVICELOST or D3DERR_DEVICENOTRESET - the user has to reset the device.
	NVAPI_INVALID_CONFIGURATION                 = -180,    //!< The requested action cannot be performed in the current state.
	NVAPI_STEREO_HANDSHAKE_NOT_DONE             = -181,    //!< Call failed as stereo handshake not completed.
	NVAPI_EXECUTABLE_PATH_IS_AMBIGUOUS          = -182,    //!< The path provided was too short to determine the correct NVDRS_APPLICATION
	NVAPI_DEFAULT_STEREO_PROFILE_IS_NOT_DEFINED = -183,    //!< Default stereo profile is not currently defined
	NVAPI_DEFAULT_STEREO_PROFILE_DOES_NOT_EXIST = -184,    //!< Default stereo profile does not exist
	NVAPI_CLUSTER_ALREADY_EXISTS                = -185,    //!< A cluster is already defined with the given configuration.
	NVAPI_DPMST_DISPLAY_ID_EXPECTED             = -186,    //!< The input display id is not that of a multi stream enabled connector or a display device in a multi stream topology
	NVAPI_INVALID_DISPLAY_ID                    = -187,    //!< The input display id is not valid or the monitor associated to it does not support the current operation
	NVAPI_STREAM_IS_OUT_OF_SYNC                 = -188,    //!< While playing secure audio stream, stream goes out of sync
	NVAPI_INCOMPATIBLE_AUDIO_DRIVER             = -189,    //!< Older audio driver version than required
	NVAPI_VALUE_ALREADY_SET                     = -190,    //!< Value already set, setting again not allowed.
	NVAPI_TIMEOUT                               = -191,    //!< Requested operation timed out
	NVAPI_GPU_WORKSTATION_FEATURE_INCOMPLETE    = -192,    //!< The requested workstation feature set has incomplete driver internal allocation resources
	NVAPI_STEREO_INIT_ACTIVATION_NOT_DONE       = -193,    //!< Call failed because InitActivation was not called.
	NVAPI_SYNC_NOT_ACTIVE                       = -194,    //!< The requested action cannot be performed without Sync being enabled.
	NVAPI_SYNC_MASTER_NOT_FOUND                 = -195,    //!< The requested action cannot be performed without Sync Master being enabled.
	NVAPI_INVALID_SYNC_TOPOLOGY                 = -196,    //!< Invalid displays passed in the NV_GSYNC_DISPLAY pointer.
	NVAPI_ECID_SIGN_ALGO_UNSUPPORTED            = -197,    //!< The specified signing algorithm is not supported. Either an incorrect value was entered or the current installed driver/hardware does not support the input value.
	NVAPI_ECID_KEY_VERIFICATION_FAILED          = -198,    //!< The encrypted public key verification has failed.
} NvAPI_Status;

#define NV_DECLARE_HANDLE(name)   struct name##__ { int unused; }; typedef struct name##__ *name
#define MAKE_NVAPI_VERSION(typeName, ver)   (NvU32)(sizeof(typeName) | ((ver)<<16))

NV_DECLARE_HANDLE(NvDRSSessionHandle);
NV_DECLARE_HANDLE(NvDRSProfileHandle);

typedef unsigned long NvU32;
typedef unsigned short NvU16;
typedef unsigned char NvU8;
typedef NvU16 NvAPI_UnicodeString[NVAPI_UNICODE_STRING_MAX];
typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];

typedef struct _NVDRS_GPU_SUPPORT
{
	NvU32 geforce    :  1;
	NvU32 quadro     :  1;
	NvU32 nvs        :  1;
	NvU32 reserved4  :  1;
	NvU32 reserved5  :  1;
	NvU32 reserved6  :  1;
	NvU32 reserved7  :  1;
	NvU32 reserved8  :  1;
	NvU32 reserved9  :  1;
	NvU32 reserved10 :  1;
	NvU32 reserved11 :  1;
	NvU32 reserved12 :  1;
	NvU32 reserved13 :  1;
	NvU32 reserved14 :  1;
	NvU32 reserved15 :  1;
	NvU32 reserved16 :  1;
	NvU32 reserved17 :  1;
	NvU32 reserved18 :  1;
	NvU32 reserved19 :  1;
	NvU32 reserved20 :  1;
	NvU32 reserved21 :  1;
	NvU32 reserved22 :  1;
	NvU32 reserved23 :  1;
	NvU32 reserved24 :  1;
	NvU32 reserved25 :  1;
	NvU32 reserved26 :  1;
	NvU32 reserved27 :  1;
	NvU32 reserved28 :  1;
	NvU32 reserved29 :  1;
	NvU32 reserved30 :  1;
	NvU32 reserved31 :  1;
	NvU32 reserved32 :  1;
} NVDRS_GPU_SUPPORT;

typedef struct _NVDRS_PROFILE
{
	NvU32                      version;            //!< Structure Version
	NvAPI_UnicodeString        profileName;        //!< String name of the Profile
	NVDRS_GPU_SUPPORT          gpuSupport;         //!< This read-only flag indicates the profile support on either
	                                               //!< Quadro, or Geforce, or both.
	NvU32                      isPredefined;       //!< Is the Profile user-defined, or predefined
	NvU32                      numOfApps;          //!< Total number of applications that belong to this profile. Read-only
	NvU32                      numOfSettings;      //!< Total number of settings applied for this Profile. Read-only
} NVDRS_PROFILE;

typedef struct _NVDRS_APPLICATION
{
	NvU32                      version;            //!< Structure Version
	NvU32                      isPredefined;       //!< Is the application userdefined/predefined
	NvAPI_UnicodeString        appName;            //!< String name of the Application
	NvAPI_UnicodeString        userFriendlyName;   //!< UserFriendly name of the Application
	NvAPI_UnicodeString        launcher;           //!< Indicates the name (if any) of the launcher that starts the application  
} NVDRS_APPLICATION;

typedef enum _NVDRS_SETTING_TYPE
{
	NVDRS_DWORD_TYPE,
	NVDRS_BINARY_TYPE,
	NVDRS_STRING_TYPE,
	NVDRS_WSTRING_TYPE
} NVDRS_SETTING_TYPE;

typedef enum _NVDRS_SETTING_LOCATION
{
	NVDRS_CURRENT_PROFILE_LOCATION,
	NVDRS_GLOBAL_PROFILE_LOCATION,
	NVDRS_BASE_PROFILE_LOCATION,
	NVDRS_DEFAULT_PROFILE_LOCATION
} NVDRS_SETTING_LOCATION;


typedef struct _NVDRS_BINARY_SETTING
{
	NvU32                valueLength;                     //!< valueLength should always be in number of bytes.
	NvU8                 valueData[NVAPI_BINARY_DATA_MAX];
} NVDRS_BINARY_SETTING;

typedef struct _NVDRS_SETTING
{
	NvU32                          version;               //!< Structure Version
	NvAPI_UnicodeString            settingName;           //!< String name of setting
	NvU32                          settingId;             //!< 32 bit setting Id
	NVDRS_SETTING_TYPE             settingType;           //!< Type of setting value.
	NVDRS_SETTING_LOCATION         settingLocation;       //!< Describes where the value in CurrentValue comes from.
	NvU32                          isCurrentPredefined;   //!< It is different than 0 if the currentValue is a predefined Value, 
	                                                      //!< 0 if the currentValue is a user value.
	NvU32                          isPredefinedValid;     //!< It is different than 0 if the PredefinedValue union contains a valid value. 
	union                                                 //!< Setting can hold either DWORD or Binary value or string. Not mixed types.
	{
		NvU32                      u32PredefinedValue;    //!< Accessing default DWORD value of this setting.
		NVDRS_BINARY_SETTING       binaryPredefinedValue; //!< Accessing default Binary value of this setting.
		                                                  //!< Must be allocated by caller with valueLength specifying buffer size, 
		                                                  //!< or only valueLength will be filled in.
		NvAPI_UnicodeString        wszPredefinedValue;    //!< Accessing default unicode string value of this setting.
	};
	union                                                 //!< Setting can hold either DWORD or Binary value or string. Not mixed types.
	{
		NvU32                      u32CurrentValue;       //!< Accessing current DWORD value of this setting.
		NVDRS_BINARY_SETTING       binaryCurrentValue;    //!< Accessing current Binary value of this setting.
		                                                  //!< Must be allocated by caller with valueLength specifying buffer size, 
		                                                  //!< or only valueLength will be filled in.
		NvAPI_UnicodeString        wszCurrentValue;       //!< Accessing current unicode string value of this setting.
};
} NVDRS_SETTING;


#define NVDRS_APPLICATION_VER   MAKE_NVAPI_VERSION(NVDRS_APPLICATION, 1)
#define NVDRS_PROFILE_VER       MAKE_NVAPI_VERSION(NVDRS_PROFILE, 1)
#define NVDRS_SETTING_VER       MAKE_NVAPI_VERSION(NVDRS_SETTING, 1)

// function pointer types
typedef NvAPI_Status *(*NvAPI_QueryInterface_t)(unsigned int offset);
typedef NvAPI_Status (*NvAPI_Initialize_t)();
typedef NvAPI_Status (*NvAPI_EnumPhysicalGPUs_t)(int **handles, int *count);
typedef NvAPI_Status (*NvAPI_GPU_GetUsages_t)(int *handle, unsigned int *usages);
typedef NvAPI_Status (*NvAPI_DRS_CreateProfile_t)(NvDRSSessionHandle hSession, NVDRS_PROFILE *pProfileInfo, NvDRSProfileHandle *phProfile);
typedef NvAPI_Status (*NvAPI_DRS_DeleteProfile_t)(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile);
typedef NvAPI_Status (*NvAPI_DRS_CreateSession_t)(NvDRSSessionHandle *phSession);
typedef NvAPI_Status (*NvAPI_DRS_LoadSettings_t)(NvDRSSessionHandle 	hSession);
typedef NvAPI_Status (*NvAPI_DRS_CreateApplication_t)(NvDRSSessionHandle hSession, NvDRSProfileHandle  hProfile, NVDRS_APPLICATION *pApplication);
typedef NvAPI_Status (*NvAPI_DRS_SetSetting_t)(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_SETTING *pSetting);
typedef NvAPI_Status (*NvAPI_DRS_SaveSettings_t)(NvDRSSessionHandle hSession);
typedef NvAPI_Status (*NvAPI_DRS_DestroySession_t)(NvDRSSessionHandle hSession);
typedef NvAPI_Status (*NvAPI_DRS_FindProfileByName_t)(NvDRSSessionHandle hSession, NvAPI_UnicodeString profileName, NvDRSProfileHandle* phProfile);
typedef NvAPI_Status (*NvAPI_DRS_FindApplicationByName_t)(NvDRSSessionHandle hSession, NvAPI_UnicodeString appName, NvDRSProfileHandle* phProfile, NVDRS_APPLICATION* pApplication);
typedef NvAPI_Status (*NvAPI_DRS_GetProfileInfo_t)(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_PROFILE* pProfileInfo);
typedef NvAPI_Status (*NvAPI_GetErrorMessage_t)(NvAPI_Status nr,NvAPI_ShortString szDesc);

// nvapi.dll internal function pointers
NvAPI_QueryInterface_t			NvAPI_QueryInterface		= NULL;
NvAPI_Initialize_t				NvAPI_Initialize			= NULL;
NvAPI_EnumPhysicalGPUs_t		NvAPI_EnumPhysicalGPUs		= NULL;
NvAPI_GPU_GetUsages_t			NvAPI_GPU_GetUsages			= NULL;
NvAPI_DRS_CreateProfile_t		NvAPI_DRS_CreateProfile		= NULL;
NvAPI_DRS_DeleteProfile_t		NvAPI_DRS_DeleteProfile		= NULL;
NvAPI_DRS_CreateSession_t		NvAPI_DRS_CreateSession		= NULL;
NvAPI_DRS_LoadSettings_t		NvAPI_DRS_LoadSettings		= NULL;
NvAPI_DRS_CreateApplication_t	NvAPI_DRS_CreateApplication	= NULL;
NvAPI_DRS_SetSetting_t			NvAPI_DRS_SetSetting 		= NULL;
NvAPI_DRS_SaveSettings_t		NvAPI_DRS_SaveSettings		= NULL;
NvAPI_DRS_DestroySession_t		NvAPI_DRS_DestroySession	= NULL;
NvAPI_DRS_FindProfileByName_t	NvAPI_DRS_FindProfileByName	= NULL;
NvAPI_DRS_FindApplicationByName_t	NvAPI_DRS_FindApplicationByName = NULL;
NvAPI_DRS_GetProfileInfo_t		NvAPI_DRS_GetProfileInfo 	= NULL;
NvAPI_GetErrorMessage_t			NvAPI_GetErrorMessage		= NULL;

}
