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
#include "fe_base.hpp"
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

FeNetTask::FeNetTask( const FeNetTask &o )
{
	*this = o;
}

const FeNetTask &FeNetTask::operator=( const FeNetTask &o )
{
	m_type = o.m_type;
	m_host = o.m_host;
	m_req = o.m_req;
	m_filename = o.m_filename;
	m_result = o.m_result;
	m_id = o.m_id;
}

bool FeNetTask::do_task( sf::Http::Response::Status &status, bool &in_req )
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

	// This call can get hung up, so we use in_req to track if we need to
	// do a hard reset of this thread in the event the user cancels while
	// it is blocked on the sendRequest call.
	//
	in_req = true;
	sf::Http::Response resp = http.sendRequest( req, sf::seconds( 8 ) );
	in_req = false;

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
			FeLog() << " ! Unable to open file for writing: "
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
	std::string &err_req, bool &in_req )
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
	bool res = t.do_task( status, in_req );

	// Queue result
	//
	{
		sf::Lock l( m_mutex );

		if ( res )
			m_out_queue.push( t );
		else if ( status != sf::Http::Response::Ok )
			err_req = t.get_req();

		m_in_flight--;

		FeDebug() << "WORKERS: queue_in=" << m_in_queue.size() << ", in_progress=" << m_in_flight
			<< ", queue_out=" << m_out_queue.size() << std::endl;
	}

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

bool FeNetQueue::all_done()
{
	sf::Lock l( m_mutex );

	bool retval = ( m_in_queue.empty() && m_out_queue.empty() && ( m_in_flight == 0 ) );
	return retval;
}

bool FeNetQueue::output_done()
{
	sf::Lock l( m_mutex );

	bool retval = ( m_out_queue.empty() && ( m_in_flight == 0 ) );
	return retval;
}

FeNetWorker::FeNetWorker( FeNetQueue &queue )
	: m_queue( queue ),
	m_thread( &FeNetWorker::work_process, this ),
	m_proceed( true ),
	m_in_req( false )
{
	m_thread.launch();
}

FeNetWorker::~FeNetWorker()
{
	// Note m_proceed and m_in_req being used across threads without mutex
	m_proceed = false;

	if ( m_in_req )
	{
		// We terminate because http.sendRequest() can hung up.
		// By terminating here the user can still escape out of a hung up scrape
		//
		FeLog() << "Hard termination of worker thread!" << std::endl;
		m_thread.terminate();
		m_in_req = false;
	}
	else
		m_thread.wait();
}

void FeNetWorker::work_process()
{
	while ( m_proceed && !m_queue.all_done() )
	{
		sf::Http::Response::Status status;
		std::string err_req;

		bool completed = m_queue.do_next_task( status, err_req, m_in_req );

		if (( status != sf::Http::Response::Ok )
			&& ( status != sf::Http::Response::NotFound ))
		{
			FeLog() << " ! Error processing request. Status code: "
				<< status << " (" << err_req << ")" << std::endl;
		}

		if ( !completed ) // sleep if there is nothing in the queue
			sf::sleep( sf::milliseconds( 10 ) );
	}

	FeDebug() << "WORKER thread process completed." << std::endl;
}
