﻿#include "stdafx.h"

#include "Common/BufferUtils.h"
#include "Emu/System.h"
#include "RSXOffload.h"

#include <thread>

namespace rsx
{
	// initialization
	void dma_manager::init()
	{
		m_worker_state = thread_state::created;
		m_enqueued_count.store(0);
		m_processed_count = 0;

		// Empty work queue in case of stale contents
		m_work_queue.pop_all();

		thread_ctrl::spawn("RSX offloader", [this]()
		{
			if (!g_cfg.video.multithreaded_rsx)
			{
				// Abort
				return;
			}

			if (g_cfg.core.thread_scheduler_enabled)
			{
				thread_ctrl::set_thread_affinity_mask(thread_ctrl::get_affinity_mask(thread_class::rsx));
			}

			while (m_worker_state != thread_state::finished)
			{
				if (m_enqueued_count.load() != m_processed_count)
				{
					for (auto slice = m_work_queue.pop_all(); slice; slice.pop_front())
					{
						auto task = *slice;
						switch (task.type)
						{
						case raw_copy:
							memcpy(task.dst, task.src, task.length);
							break;
						case vector_copy:
							memcpy(task.dst, task.opt_storage.data(), task.length);
							break;
						case index_emulate:
							write_index_array_for_non_indexed_non_native_primitive_to_buffer(
								reinterpret_cast<char*>(task.dst),
								static_cast<rsx::primitive_type>(task.aux_param0),
								task.length);
							break;
						default:
							ASSUME(0);
							fmt::throw_exception("Unreachable" HERE);
						}

						++m_processed_count;
					}
				}
				else
				{
					// Yield
					std::this_thread::yield();
				}
			}

			m_processed_count = m_enqueued_count.load();
		});
	}

	// General transport
	void dma_manager::copy(void *dst, std::vector<u8>& src, u32 length)
	{
		if (length <= max_immediate_transfer_size || !g_cfg.video.multithreaded_rsx)
		{
			std::memcpy(dst, src.data(), length);
		}
		else
		{
			++m_enqueued_count;
			m_work_queue.push(dst, src, length);
		}
	}

	void dma_manager::copy(void *dst, void *src, u32 length)
	{
		if (length <= max_immediate_transfer_size || !g_cfg.video.multithreaded_rsx)
		{
			std::memcpy(dst, src, length);
		}
		else
		{
			++m_enqueued_count;
			m_work_queue.push(dst, src, length);
		}
	}

	// Vertex utilities
	void dma_manager::emulate_as_indexed(void *dst, rsx::primitive_type primitive, u32 count)
	{
		if (!g_cfg.video.multithreaded_rsx)
		{
			write_index_array_for_non_indexed_non_native_primitive_to_buffer(
				reinterpret_cast<char*>(dst), primitive, count);
		}
		else
		{
			++m_enqueued_count;
			m_work_queue.push(dst, primitive, count);
		}
	}

	// Synchronization
	void dma_manager::sync()
	{
		if (LIKELY(m_enqueued_count.load() == m_processed_count))
		{
			// Nothing to do
			return;
		}

		while (m_enqueued_count.load() != m_processed_count)
			_mm_lfence();
	}

	void dma_manager::join()
	{
		m_worker_state = thread_state::finished;
		sync();
	}
}
