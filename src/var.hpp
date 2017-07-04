#pragma once
/*
* Covariant Script: Var
* This program is based on Covariant Mozart.
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
#include "./exceptions.hpp"
#include "./memory.hpp"
#include <functional>

namespace cs {
	constexpr std::size_t cs_var_pool_size=96;
	template<typename _Tp> class compare_helper {
		template<typename T,typename X=bool>struct matcher;
		template<typename T> static constexpr bool match(T*)
		{
			return false;
		}
		template<typename T> static constexpr bool match(matcher < T, decltype(std::declval<T>()==std::declval<T>()) > *)
		{
			return true;
		}
	public:
		static constexpr bool value = match<_Tp>(nullptr);
	};
	template<typename,bool> struct compare_if;
	template<typename T>struct compare_if<T,true> {
		static bool compare(const T& a,const T& b)
		{
			return a==b;
		}
	};
	template<typename T>struct compare_if<T,false> {
		static bool compare(const T&,const T&)
		{
			throw lang_error("CSLE0001");
		}
	};
	template<typename T>bool compare(const T& a,const T& b)
	{
		return compare_if<T,compare_helper<T>::value>::compare(a,b);
	}
	template<typename _Tp> class to_string_helper {
		template<typename T,typename X>struct matcher;
		template<typename T> static constexpr bool match(T*)
		{
			return false;
		}
		template<typename T> static constexpr bool match(matcher<T,decltype(std::to_string(std::declval<T>()))>*)
		{
			return true;
		}
	public:
		static constexpr bool value = match<_Tp>(nullptr);
	};
	template<typename,bool> struct to_string_if;
	template<typename T>struct to_string_if<T,true> {
		static std::string to_string(const T& val)
		{
			return std::to_string(val);
		}
	};
	template<typename T>struct to_string_if<T,false> {
		static std::string to_string(const T&)
		{
			throw lang_error("CSLE0002");
		}
	};
	template<typename T>std::string to_string(const T& val)
	{
		return to_string_if<T,to_string_helper<T>::value>::to_string(val);
	}
	template<typename _Tp> class hash_helper {
		template<typename T,decltype(&std::hash<T>::operator()) X>struct matcher;
		template<typename T> static constexpr bool match(T*)
		{
			return false;
		}
		template<typename T> static constexpr bool match(matcher<T,&std::hash<T>::operator()>*)
		{
			return true;
		}
	public:
		static constexpr bool value = match<_Tp>(nullptr);
	};
	template<typename,bool> struct hash_if;
	template<typename T>struct hash_if<T,true> {
		static std::size_t hash(const T& val)
		{
			static std::hash<T> gen;
			return gen(val);
		}
	};
	template<typename T>struct hash_if<T,false> {
		static std::size_t hash(const T& val)
		{
			throw lang_error("CSLE0003");
		}
	};
	template<typename T>std::size_t hash(const T& val)
	{
		return hash_if<T,hash_helper<T>::value>::hash(val);
	}
	class var final {
		class baseHolder {
		public:
			baseHolder() = default;
			virtual ~ baseHolder() = default;
			virtual const std::type_info& type() const = 0;
			virtual baseHolder* duplicate() = 0;
			virtual bool compare(const baseHolder *) const = 0;
			virtual std::string to_string() const = 0;
			virtual std::size_t hash() const = 0;
			virtual void kill() = 0;
		};
		template<typename T>class holder:public baseHolder {
		protected:
			T mDat;
		public:
			static cov::allocator<holder<T>,cs_var_pool_size> allocator;
			holder() = default;
			template<typename...ArgsT>holder(ArgsT&&...args):mDat(std::forward<ArgsT>(args)...) {}
			virtual ~ holder() = default;
			virtual const std::type_info& type() const override
			{
				return typeid(T);
			}
			virtual baseHolder* duplicate() override
			{
				return allocator.alloc(mDat);
			}
			virtual bool compare(const baseHolder* obj) const override
			{
				if (obj->type()==this->type()) {
					const holder<T>* ptr=dynamic_cast<const holder<T>*>(obj);
					return ptr!=nullptr?cs::compare(mDat,ptr->data()):false;
				}
				return false;
			}
			virtual std::string to_string() const override
			{
				return cs::to_string(mDat);
			}
			virtual std::size_t hash() const override
			{
				return cs::hash<T>(mDat);
			}
			virtual void kill() override
			{
				allocator.free(this);
			}
			T& data()
			{
				return mDat;
			}
			const T& data() const
			{
				return mDat;
			}
			void data(const T& dat)
			{
				mDat = dat;
			}
		};
		baseHolder* mDat=nullptr;
		var(baseHolder* ptr):mDat(ptr) {}
	public:
		void swap(var& obj)
		{
			std::swap(this->mDat,obj.mDat);
		}
		void swap(var&& obj) noexcept
		{
			std::swap(this->mDat,obj.mDat);
		}
		bool usable() const noexcept
		{
			return mDat!=nullptr;
		}
		template<typename T,typename...ArgsT>static var make(ArgsT&&...args)
		{
			return var(holder<T>::allocator.alloc(std::forward<ArgsT>(args)...));
		}
		var()=default;
		template<typename T> explicit var(const T & dat):mDat(holder<T>::allocator.alloc(dat)) {}
		var(const var& v):mDat(v.mDat==nullptr?nullptr:v.mDat->duplicate()) {}
		var(var&& v) noexcept
		{
			swap(std::forward<var>(v));
		}
		~var()
		{
			delete mDat;
		}
		const std::type_info& type() const
		{
			return this->mDat!=nullptr?this->mDat->type():typeid(void);
		}
		std::string to_string() const
		{
			if(this->mDat==nullptr)
				return "Null";
			return this->mDat->to_string();
		}
		std::size_t hash() const
		{
			if(this->mDat==nullptr)
				return cs::hash<void*>(nullptr);
			return this->mDat->hash();
		}
		var& operator=(const var& v)
		{
			if(&v!=this) {
				if(mDat!=nullptr)
					mDat->kill();
				mDat=v.mDat==nullptr?nullptr:v.mDat->duplicate();
			}
			return *this;
		}
		var& operator=(var&& v) noexcept
		{
			swap(std::forward<var>(v));
			return *this;
		}
		template<typename T> var& operator=(const T& dat)
		{
			if(mDat!=nullptr)
				mDat->kill();
			mDat=holder<T>::allocator.alloc(dat);
			return *this;
		}
		bool operator==(const var& v) const
		{
			return usable()?this->mDat->compare(v.mDat):!v.usable();
		}
		bool operator!=(const var& v)const
		{
			return usable()?!this->mDat->compare(v.mDat):v.usable();
		}
		template<typename T> T& val() const
		{
			if(typeid(T)!=this->type())
				throw lang_error("CSLE0006");
			if(this->mDat==nullptr)
				throw lang_error("CSLE0005");
			return dynamic_cast<const holder<T>*>(this->mDat)->data();
		}
		template<typename T> operator T&() const
		{
			return this->val<T>();
		}
	};
	template<> std::string to_string<std::string>(const std::string& str)
	{
		return str;
	}
	template<> std::string to_string<bool>(const bool& v)
	{
		if(v)
			return "true";
		else
			return "false";
	}
	template<typename T> cov::allocator<var::holder<T>,cs_var_pool_size> var::holder<T>::allocator;
	template<int N> class var::holder<char[N]>:public var::holder<std::string> {
	public:
		using holder<std::string>::holder;
	};
	template<> class var::holder<std::type_info>:public var::holder<std::type_index> {
	public:
		using holder<std::type_index>::holder;
	};
}
