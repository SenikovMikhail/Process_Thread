#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>  
#include <list>
#include <thread>

#include <Windows.h>


void* i(int argc, void** argv) {
	std::cout << *((int*)argv[0]) << std::endl;

	return nullptr;
}

void* c(int argc, void** argv) {
	Sleep(2000);
	std::cout << *((char*)argv[0]) << std::endl;
	return nullptr;
}

void* g(int argc, void** argv) {
	std::cout << *((char*)argv[1]) << std::endl;

	return nullptr;
}

void* t(int argc, void** argv) {
	Sleep(5000);
	std::cout << "pr2" << std::endl;

	return nullptr;
}

class desc {

public:

	std::string func_name;
	int argc;
	void** argv;
	void* (*func)(int, void**);
	desc(std::string fn_name, void* (*fn)(int, void**), int arc, void** arg) {
		func_name = fn_name;
		func = fn;
		argc = arc;
		argv = arg;
	}
};

class Environment {

public:
	std::vector<desc*> func_l = {};

	Environment& operator+= (desc* desc_fun) {
		func_l.push_back(desc_fun);
		return *this;
	}

};

class ethread {

public:

	int _id;
	int _priority; // 0 <= ... <= 4
	int _status = 0;
	desc* _func_description;

	ethread() {
		_id = -1;
		_priority = -1;
		_status = 1;
		_func_description = nullptr;
	}

	ethread(int id, int priority, desc* func_description)
		: _id(id)
		, _priority(priority)
		, _func_description(func_description)
	{}


	ethread& operator =(const ethread& other) {
		_id = other._id;
		_priority = other._priority;
		_status = other._status;
		_func_description = other._func_description;

		return *this;
	}

	ethread& operator =(ethread& other) {
		_id = other._id;
		_priority = other._priority;
		_status = other._status;
		_func_description = other._func_description;

		other._func_description = nullptr;

		return *this;
	}

	void exec(int *status) {

		(*_func_description->func)(_func_description->argc, _func_description->argv);
		*status = 1;
	}
	
};

class eprocess {

public:

	int id;
	int priority; // 0 <= ... <= 4
	Environment* PEB;
	std::unordered_map<std::string, desc*> tr_um;
	std::list<ethread> tr_arr[5];
	int tr_cout = 0;

	eprocess(int p)
		: priority(p)
	{}

	eprocess& operator+= (Environment* pb) {
		PEB = pb;

		for (unsigned int i = 0; i < PEB->func_l.size(); ++i) {
			tr_um.insert(std::make_pair(PEB->func_l[i]->func_name, PEB->func_l[i]));
		}

		return *this;
	}

	void create_tr(std::string func_name, int priority) {
		int tr_count = 0;
		for (int i = 0; i < 5; ++i)
			tr_cout += tr_arr[i].size();
		ethread tr(tr_count, priority, tr_um.at(func_name));
		tr_arr[priority].push_back(tr);
	}
};

class scheduler {

public:

	std::list<ethread> tr_arr[9];

	scheduler& operator +=(eprocess pr) {

		for (int i = 0; i < 5; ++i)
			if (!(pr.tr_arr[i].empty())) {
				for (ethread tr : pr.tr_arr[i])
					tr_arr[pr.priority + tr._priority].push_back(tr);

			}

		return *this;
	}


	ethread get_tr() {

		for (int i = 8; i >= 0; --i) {
			if (!(tr_arr[i].empty())) {
				ethread ret_tr = tr_arr[i].front();
				tr_arr[i].pop_front();
				return ret_tr;
			}
		}

	}

};


class cpu{

public:

	int kernel_count = 10;
	ethread* tr_arr = new ethread[kernel_count];
	scheduler *sc;
	int i = 0;
	int tr_count = 0;

	cpu(scheduler* sch) {

		sc = sch;

		for (int i = 0; i < 9; ++i) {
			if (!sc->tr_arr[i].empty())
				tr_count += sc->tr_arr[i].size();
		}

	}

	void exec() {

		for (int itr = 0; itr < tr_count; ++itr) {

			ethread	 tr = sc->get_tr();

			i = 0;
			while (tr_arr[i]._status != 1) {
				++i;
				i %= kernel_count;

			}

			tr_arr[i] = tr;
			std::cout << "~~~~~ " << i << " ~~~~~" << std::endl;
			
			std::thread kr(&ethread::exec, tr_arr[i], &(tr_arr[i]._status) );
			kr.detach();
			std::cout << std::endl;
		}
	}
};

int main() {



	std::string func_name = "test1";
	void* (*func_ptr)(int, void**) = &i;
	int a = 5;
	void* arg[] = { &a };

	desc ob(func_name, func_ptr, 1, arg);

	func_name = "test2";
	func_ptr = &c;
	char ch = 't';
	void* argv2[]{ &ch };

	desc ob2(func_name, func_ptr, 1, argv2);

	func_name = "test3";
	func_ptr = &g;
	char ch2 = 'e';
	void* argv3[] = { &ch, &ch2 };

	desc ob3(func_name, func_ptr, 2, argv3);

	Environment env;
	env += &ob;
	env += &ob2;
	env += &ob3;

	//---------------------------------------------------------------

	eprocess pr(1);
	pr += &env;

	pr.create_tr("test1", 1);
	pr.create_tr("test2", 2);
	pr.create_tr("test1", 3);
	pr.create_tr("test3", 3);

	//---------------------------------------------------------------

	func_name = "test4";
	func_ptr = &t;
	void** argv4 = nullptr;

	desc ob4(func_name, func_ptr, 0, argv4);

	Environment env2;
	env2 += &ob4;

	//---------------------------------------------------------------

	eprocess pr2(4);
	pr2 += &env2;

	pr2.create_tr("test4", 4);


	//---------------------------------------------------------------

	scheduler sc;

	sc += pr;
	sc += pr2;

	cpu cp(&sc);
	cp.exec();


	system("pause");

	return 0;
}