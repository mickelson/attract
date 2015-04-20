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

#ifndef FE_NET_HPP
#define FE_NET_HPP

#include <SFML/Network.hpp>
#include <queue>

class FeNetTask
{
public:
	enum TaskType
	{
		FeNetNoTask,
		FeNetFileTask,
		FeNetBufferTask
	};

	FeNetTask(
		const std::string &host,
		const std::string &req,
		const std::string &filename );

	FeNetTask(
		const std::string &host,
		const std::string &req,
		int id );

	FeNetTask();

	bool do_task( sf::Http::Response::Status &status );
	void get_result( int &id, std::string &result );

	std::string &get_req() { return m_req; };
private:
	TaskType m_type;
	std::string m_host;
	std::string m_req;
	std::string m_filename;
	std::string m_result;
	int m_id;
};

class FeNetQueue
{
private:
	sf::Mutex m_mutex;
	std::queue < FeNetTask > m_in_queue;
	std::queue < FeNetTask > m_out_queue;
	int m_in_flight;

	FeNetQueue( const FeNetQueue & );
	FeNetQueue &operator=( const FeNetQueue & );

public:
	FeNetQueue();

	void add_file_task( const std::string &host,
			const std::string &req,
			const std::string &file_name );

	void add_buffer_task( const std::string &host,
			const std::string &req,
			int id );

	bool do_next_task( sf::Http::Response::Status &status );

	bool pop_completed_task( int &id,
			std::string &result );

	bool input_done();
	bool output_done();
};

class FeNetWorker
{
	FeNetQueue &m_queue;
	sf::Thread m_thread;
	bool m_proceed;

	FeNetWorker( const FeNetWorker & );
	FeNetWorker &operator=( const FeNetWorker & );

	void work_process();
public:
	FeNetWorker( FeNetQueue &q );
	~FeNetWorker();
};

#endif
