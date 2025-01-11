#include <iostream>
#include "exec.hpp"

int main(int argc, char** argv) {

	std::cout << "executing /bin/ls" << std::endl;
	exec e1 = { 
		.cmd = "/bin/ls",
		.fork = true,
		.hooks = {
			.before = [](exec& ex) { std::cout << "child process started with pid " << ex.status.pid() << std::endl; },
			.after = [](exec& ex) { std::cout << "child process " << ex.status.pid() << " ended with status code " << ex.status.code() << std::endl; }
		}
	};
	int i = e1.perform();

	std::cout << "command return value: " << i << std::endl;

	std::cout << "executing shell" << std::endl;
	e1 = {
		.cmd = "/bin/sh",
		.env = {{ "hello", "world" }},
		.copy_env = true,
		.fork = true, .wait = true
	};
	e1.perform();

	

	std::cout << "program ends, return value of previous exec: " << e1.status.code() << std::endl;

	return 0;
}
