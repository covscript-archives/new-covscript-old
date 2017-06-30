#pragma once
/*
*	Covariant Script Core
*	This Source Code Form is subject to the terms of the Mozilla Public
*	License, v. 2.0. If a copy of the MPL was not distributed with this
*	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <string>
#include <deque>
#include "./exceptions.hpp"
#include "./memory.hpp"
#include "./var.hpp"
#include "./cni.hpp"
namespace cs {
// Type definition
	using integer=long;
	using floating=long double;
	using character=char;
	using boolean=bool;
	using literal=std::string;
// Version
	const string version="1.0.0";
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
		ready,bisy,idle,finish
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
		std::size_t mPosit=0;
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
			mIns=line;
		}
		void call(virtual_machine* vm)
		{
			if(mStatus!=thread_status::ready)
				throw cs::lang_error("CSLE0001");
			for(; mPosit<mIns.size(); ++mPosit)
				mIns.at(mPosit)->exec(vm,this);
			mPosit=0;
		}
		void exec()
		{
			if(mStatus==thread_status::finish)
				throw cs::lang_error("CSLE0002");
			mIns.at(mPosit)->exec(vm,this);
			if(++mPosit>=mIns.size())
				mStatus=thread_status::finish;
		}
	};
	class virtual_machine final {
		using var_pointer_t=cov::storage<var,var_pool_size>::pointer;
		using thread_pointer_t=cov::storage<thread,thread_pool_size>::pointer;
		cov::storage<var,var_pool_size> var_pool;
		cov::storage<thread,thread_pool_size> thread_pool;
	public:
		virtual_machine()=default;
		virtual_machine(const virtual_machine&)=delete;
		~virtual_machine()=default;
		thread_pointer_t create_thread(const std::deque<instruction_base*>& ins)
		{
			return thread_pool.alloc(ins);
		}
	};
}
