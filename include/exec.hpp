#pragma once
#include <functional>
#include <string>
#include <vector>
#include <map>

struct exec {

	public:

		struct result {

			friend exec;

			public:
				pid_t pid();
				int code();

			private:
				pid_t _pid;
				int _code;
				result(pid_t pid, int code) : _pid(pid), _code(code) {}
		};

		struct functions {
			std::function<void(exec&)> before = {};
			std::function<void(exec&)> after = {};
		};

		std::string cmd;
		std::vector<std::string> args;
		std::map<std::string, std::string> env;
		bool copy_env = true;
		bool fork = false;
		bool wait = true;
		functions hooks;

		exec::result status = exec::result(-1, -1);
		exec& operator =(const exec& other);

		int perform();
};
