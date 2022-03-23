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

#include <mutex>
#include <thread>
#include <deque>
#include <queue>

class FeNetWorker;

class FeNetTask
{
public:
	enum TaskType
	{
		NoTask=0,
		FileTask=-1,
		SpecialFileTask=-2,
		FileTaskError=-3,
		BufferTask=-4
	};

	FeNetTask( const FeNetTask & );
	const FeNetTask &operator=( const FeNetTask & );

	FeNetTask(
		const std::string &url,
		const std::string &filename,
		TaskType t=FileTask );

	FeNetTask(
		const std::string &url,
		int id );

	FeNetTask();

	bool do_task( long *code = NULL );

	// this function consumes the task's result, so it will no longer be
	// available for future calls to this function...
	void grab_result( int &id, std::string &result );

private:
	TaskType m_type;
	std::string m_url;
	std::string m_filename;
	std::string m_result;
	int m_id;
};

class FeNetQueue
{
	friend class FeNetWorker;
private:
	std::recursive_mutex m_mutex;
	std::deque < FeNetTask > m_in_queue;
	std::queue < FeNetTask > m_out_queue;
	int m_in_flight;

	FeNetQueue( const FeNetQueue & );
	FeNetQueue &operator=( const FeNetQueue & );

protected:
	bool get_next_task( FeNetTask &t );
	void done_with_task( const FeNetTask &t, bool queue_result );

	void abort();

public:
	FeNetQueue();

	void add_file_task( const std::string &url,
			const std::string &file_name,
			bool flag_special=false );

	void add_buffer_task( const std::string &url,
			int id );

	bool pop_completed_task( int &id,
			std::string &result );

	bool all_done();
	bool output_done();
};

class FeNetWorker
{
	FeNetQueue &m_queue;
	std::thread m_thread;
	bool m_proceed;

	FeNetWorker( const FeNetWorker & );
	FeNetWorker &operator=( const FeNetWorker & );

	void work_process();
public:
	FeNetWorker( FeNetQueue &q );
	~FeNetWorker();
};

#endif
