/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2015 Andrew Mickelson
 *
 *  This file is part of Attract-Mode.
 *
 *  Attract-Mode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Attract-Mode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Attract-Mode.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "fe_net.hpp"
#include <fstream>
#include <iostream>

FeNetTask::FeNetTask( const std::string &host,
		const std::string &req,
		const std::string &filename,
		TaskType t )
	: m_type( t ),
	m_host( host ),
	m_req( req ),
	m_filename( filename ),
	m_id( FileTaskError )
{
}

FeNetTask::FeNetTask( const std::string &host,
		const std::string &req, int id )
	: m_type( BufferTask ),
	m_host( host ),
	m_req( req ),
	m_id( id )
{
}


FeNetTask::FeNetTask()
	: m_type( NoTask ),
	m_id( 0 )
{
}

bool FeNetTask::do_task( sf::Http::Response::Status &status )
{
	const char *FROM_FIELD = "From";
	const char *FROM_VALUE = "user@attractmode.org";
	const char *UA_FIELD = "User-Agent";
	const char *UA_VALUE = "Attract-Mode/1.x";

	sf::Http http;
	http.setHost( m_host );

	sf::Http::Request req( m_req );
	req.setField( FROM_FIELD, FROM_VALUE );
	req.setField( UA_FIELD, UA_VALUE );

	sf::Http::Response resp = http.sendRequest( req, sf::seconds( 8 ) );
	status = resp.getStatus();

	if ( status != sf::Http::Response::Ok )
		return false;

	if (( m_type == FileTask ) || ( m_type == SpecialFileTask ))
	{
		size_t pos = m_req.find_last_of( '.' );
		if ( pos != std::string::npos )
			m_filename += m_req.substr( pos );
		else
			m_filename += ".png";

		std::ofstream outfile( m_filename.c_str(), std::ios_base::binary );
		if ( !outfile.is_open() )
		{
			std::cerr << " ! Unable to open file for writing: "
				<< m_filename << std::endl;
			return false;
		}

		outfile << resp.getBody();
		outfile.close();

		m_id=m_type;
		m_result = m_filename;
	}
	else
		m_result = resp.getBody();

	return true;
}

void FeNetTask::grab_result( int &id, std::string &result )
{
	id = m_id;
	result.swap( m_result );
}

FeNetQueue::FeNetQueue()
	: m_in_flight( 0 )
{
}

void FeNetQueue::add_file_task( const std::string &host,
		const std::string &req,
		const std::string &file_name,
		bool flag_special )
{
	sf::Lock l( m_mutex );
	m_in_queue.push_front( FeNetTask( host, req, file_name,
		flag_special ? FeNetTask::SpecialFileTask : FeNetTask::FileTask ) );
}

void FeNetQueue::add_buffer_task( const std::string &host,
		const std::string &req,
		int id )
{
	sf::Lock l( m_mutex );
	m_in_queue.push_back( FeNetTask( host, req, id ) );
}

bool FeNetQueue::do_next_task( sf::Http::Response::Status &status,
	std::string &err_req )
{
	FeNetTask t;
	status = sf::Http::Response::Ok;

	// Grab next task from the input queue
	//
	{
		sf::Lock l( m_mutex );
		if ( m_in_queue.empty() )
			return false;

		t = m_in_queue.front();
		m_in_queue.pop_front();

		m_in_flight++;
	}

	// Perform task
	//
	if ( t.do_task( status ) )
	{
		// Queue Result
		//
		sf::Lock l( m_mutex );
		m_out_queue.push( t );
		m_in_flight--;
	}
	else if ( status != sf::Http::Response::Ok )
		err_req = t.get_req();

	return true;
}

bool FeNetQueue::pop_completed_task( int &id,
		std::string &result )
{
	sf::Lock l( m_mutex );
	if ( !m_out_queue.empty() )
	{
		m_out_queue.front().grab_result( id, result );
		m_out_queue.pop();
		return true;
	}
	return false;
}

bool FeNetQueue::input_done()
{
	sf::Lock l( m_mutex );
	return m_in_queue.empty();
}

bool FeNetQueue::output_done()
{
	sf::Lock l( m_mutex );
	return ( m_out_queue.empty() && ( m_in_flight == 0 ) );
}

FeNetWorker::FeNetWorker( FeNetQueue &queue )
	: m_queue( queue ),
	m_thread( &FeNetWorker::work_process, this ),
	m_proceed( true )
{
	m_thread.launch();
}

FeNetWorker::~FeNetWorker()
{
	m_proceed = false;
	m_thread.wait();
}

void FeNetWorker::work_process()
{
	while ( !m_queue.input_done() && m_proceed )
	{
		sf::Http::Response::Status status;
		std::string err_req;

		bool completed = m_queue.do_next_task( status, err_req );

		if ( status != sf::Http::Response::Ok )
		{
			std::cerr << " ! Error processing request. Status code: "
				<< status << " (" << err_req << ")" << std::endl;
		}

		if ( !completed ) // sleep if there is nothing in the queue
			sf::sleep( sf::milliseconds( 10 ) );
	}
}
