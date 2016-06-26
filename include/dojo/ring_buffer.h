#pragma once

namespace Dojo {
	template<class T>
	class ring_buffer {
	public:
		explicit ring_buffer(size_t maxSize) : m_max_size(maxSize) {
			m_buffer.reserve(maxSize);
		}

		size_t max_size() const {
			return m_max_size;
		}

		size_t size() const {
			return m_buffer.size();
		}

		auto peek_next() const {
			if(m_write_idx < m_buffer.size()) {
				return m_buffer.begin() + m_write_idx;
			}
			return m_buffer.end();
		}

		auto begin() {
			return m_buffer.begin();
		}

		auto end() {
			return m_buffer.end();
		}

		template <class... Args>
		void emplace(Args&& ... args) {
			if (m_write_idx == m_buffer.size()) {
				m_buffer.emplace_back(std::forward<Args>(args)...);
			}
			else {
				m_buffer[m_write_idx] = T(std::forward<Args>(args)...);
			}

			m_write_idx = (m_write_idx + 1) % max_size();
		}

	private:
		std::vector<T> m_buffer;
		size_t m_write_idx = 0;
		size_t m_max_size;
	};
}

