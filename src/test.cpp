#include "./core.hpp"
#include <iostream>
class test_calc_ins final:public cs::instruction_base {
	cs::literal message;
public:
	test_calc_ins(const cs::literal& msg):message(msg) {}
	virtual cs::instruction_type type() const override
	{
		return cs::instruction_type::calc;
	}
	virtual void exec(cs::virtual_machine* vm,cs::thread* th) const override
	{
		std::cout<<message<<std::endl;
	}
};
class test_jmp_ins final:public cs::instruction_base {
public:
	virtual cs::instruction_type type() const override
	{
		return cs::instruction_type::jump;
	}
	virtual void exec(cs::virtual_machine* vm,cs::thread* th) const override
	{
		th->jump(0);
	}
};
int main()
{
	cs::virtual_machine vm;
	auto th0=vm.create_thread({new test_calc_ins("th0"),new test_jmp_ins});
	auto th1=vm.create_thread({new test_calc_ins("th1"),new test_calc_ins("Hello"),new test_jmp_ins});
	vm.join_thread(th0);
	vm.join_thread(th1);
	vm.start();
	return 0;
}
