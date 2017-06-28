#pragma once
/*
*	Covariant Script Core
*	This Source Code Form is subject to the terms of the Mozilla Public
*	License, v. 2.0. If a copy of the MPL was not distributed with this
*	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <string>
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
// Classes definition
// Instruction Enumerations
	enum class instruction_type {
		calc,tag,jump,jict,jicf
	};
// Instruction Base Class
	class instruction_base;
// Compiler Class
	class compiler;
// Virtual Machine Class
	class virtual_machine;
// Classes Realization
	class instruction_base {
	public:
		instruction_base()=default;
		instruction_base(const instruction_base&)=default;
		virtual ~instruction_base()=default;
		virtual instruction_type type() const=0;
		virtual void exec(virtual_machine&) const=0;
	};
}
