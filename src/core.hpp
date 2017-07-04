#pragma once
/*
* Covariant Script
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* Copyright (C) 2017 Michael Lee(李登淳)
* Email: mikecovlee@163.com
* Github: https://github.com/mikecovlee
*
* Version: 1.0.0
*/
#include <string>
#include <deque>
#include <list>
#include "./exceptions.hpp"
#include "./memory.hpp"
#include "./var.hpp"
namespace cs {
// Type definition
	using integer=long;
	using floating=long double;
	using character=char;
	using boolean=bool;
	using literal=std::string;
// Version
	const literal version="1.0.0";
// Memory Pool
	constexpr std::size_t var_pool_size=10240;
	constexpr std::size_t thread_pool_size=1024;
// Classes definition
// Instruction Enumerations
	enum class instruction_type {
		calc,tag,jump,jict,jicf,call,join
	};
// Thread Status Enumerations
	enum class thread_status {
		ready,busy,idle,finish
	};
// Instruction Base Class
	class instruction_base;
// Compiler Class
	class compiler;
// Thread Class
	class thread;
// Virtual Machine Class
	class virtual_machine;
// Classes Realization
	class instruction_base {
	public:
		instruction_base()=default;
		instruction_base(const instruction_base&)=default;
		virtual ~instruction_base()=default;
		virtual instruction_type type() const=0;
		virtual void exec(virtual_machine*,thread*) const=0;
	};
	class thread final {
		thread_status mStatus=thread_status::ready;
		const std::deque<instruction_base*> mIns;
		std::size_t mPosit=1;
	public:
		thread()=delete;
		thread(const std::deque<instruction_base*>& ins):mIns(ins) {}
		thread(const thread&)=delete;
		~thread()=default;
		void set_status(thread_status status) noexcept
		{
			mStatus=status;
		}
		thread_status get_status() const noexcept
		{
			return mStatus;
		}
		void jump(std::size_t line)
		{
			mPosit=line;
		}
		void call(virtual_machine* vm)
		{
			if(mStatus!=thread_status::ready)
				throw cs::lang_error("CSLE0001");
			for(; mPosit-1<mIns.size(); ++mPosit)
				mIns.at(mPosit-1)->exec(vm,this);
			mPosit=0;
		}
		void exec(virtual_machine* vm)
		{
			if(mStatus==thread_status::finish)
				throw cs::lang_error("CSLE0002");
			mIns.at(mPosit-1)->exec(vm,this);
			if(++mPosit-1>=mIns.size())
				mStatus=thread_status::finish;
		}
	};
	class virtual_machine final {
		using var_pointer_t=cov::storage<var,var_pool_size>::pointer;
		using thread_pointer_t=cov::storage<thread,thread_pool_size>::pointer;
		cov::storage<var,var_pool_size> var_pool;
		cov::storage<thread,thread_pool_size> thread_pool;
		std::list<var_pointer_t> var_free_list;
		std::list<thread_pointer_t> thread_list;
	public:
		virtual_machine()=default;
		virtual_machine(const virtual_machine&)=delete;
		~virtual_machine()=default;
		thread_pointer_t create_thread(const std::deque<instruction_base*>& ins)
		{
			return thread_pool.alloc(ins);
		}
		void free_thread(thread_pointer_t tptr)
		{
			thread_pool.free(tptr);
		}
		void join_thread(thread_pointer_t th)
		{
			if(thread_pool.get(th).get_status()!=thread_status::ready)
				throw lang_error("CSLE0003");
			thread_pool.get(th).set_status(thread_status::busy);
			thread_list.push_back(th);
		}
		var_pointer_t create_var()
		{
			if(!var_free_list.empty()) {
				var_pointer_t vptr=var_free_list.back();
				var_free_list.pop_back();
				return vptr;
			}
			else
				return var_pool.alloc();
		}
		void free_var(var_pointer_t vptr)
		{
			var_free_list.push_front(vptr);
		}
		void start()
		{
			while(!thread_list.empty()) {
				thread_list.remove_if([&](const thread_pointer_t& th) {
					return thread_pool.get(th).get_status()==thread_status::finish;
				});
				for(auto& ptr:thread_list) {
					thread& th=thread_pool.get(ptr);
					if(th.get_status()!=thread_status::idle&&th.get_status()!=thread_status::finish)
						th.exec(this);
				}
			}
		}
	};
}
