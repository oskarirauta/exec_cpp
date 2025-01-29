#include <stdexcept>
#include <algorithm>
#include <utility>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

#include "exec.hpp"

extern char **environ;

#include <iostream>

static std::string to_upper(const std::string& str) {

	std::string _str(str);

        for ( auto& ch : _str )
                if ( std::islower(ch))
                        ch &= ~32;
        return _str;
}

std::map<std::string, std::string> exec::environ() {

	if ( ::environ == nullptr )
		return {};

	std::map<std::string, std::string> ret;

	char** env = ::environ;

	for ( ; *env; ++env ) {

		std::string value(*env);
		if ( auto pos = value.find('='); pos != std::string::npos ) {
			std::string key = value.substr(0, pos);
			value.erase(0, pos + 1);
			if ( !key.empty())
				ret.emplace(key, value);
		}
	}

	return ret;
}

pid_t exec::result::pid() {

	return this -> _pid;
}

int exec::result::code() {

	return this -> _code;
}

exec& exec::operator =(const exec& other) {

	this -> cmd = other.cmd;
	this -> args = other.args;
	this -> env = other.env;
	this -> copy_env = other.copy_env;
	this -> fork = other.fork;
	this -> wait = other.wait;
	this -> hooks.before = other.hooks.before;
	this -> hooks.after = other.hooks.after;

	this -> status._pid = -1;
	this -> status._code = -1;

	return *this;
}

int exec::perform() {

	int ret;

	std::vector<const char*> exec_args = { this -> cmd.c_str() };
	std::transform(this -> args.begin(), this -> args.end(), std::back_inserter(exec_args),
			[](const std::string& s) { return s.c_str(); });
	exec_args.push_back(nullptr);

	std::vector<std::string> env_strings;
	std::vector<const char*> env_args;

	std::transform(this -> env.begin(), this -> env.end(), std::back_inserter(env_strings),
			[](const std::pair<std::string, std::string>& p) { return !p.first.empty() && !p.second.empty() ? ( p.first + "=" + p.second ) : ""; });

	if ( this -> copy_env ) {

		std::map<std::string, std::string> current_env = exec::environ();
		std::map<std::string, std::string> env_copy = this -> env;

		std::transform(current_env.begin(), current_env.end(), std::back_inserter(env_strings),
			[&env_copy](const std::pair<std::string, std::string>& p) {

			if ( std::find_if(env_copy.begin(), env_copy.end(),
				[&p](const std::pair<std::string, std::string>& e) {
					return to_upper(p.first) == to_upper(e.first);
				}) == env_copy.end() && !p.first.empty() && !p.second.empty())
				return p.first + "=" + p.second;
			else return std::string("");
		});
	}

	env_strings.erase(std::remove_if(env_strings.begin(), env_strings.end(),
					[](const std::string& s) { return s.empty(); }), env_strings.end());

	if ( !env_strings.empty()) {

		std::transform(env_strings.begin(), env_strings.end(), std::back_inserter(env_args),
				[](const std::string& s) { return s.c_str(); });

		env_args.push_back(nullptr);
	}

	this -> status._code = -1;
	this -> status._pid = ::getpid();
	pid_t child = this -> fork ? ::fork() : 0;

	if ( child == -1 )
		throw std::runtime_error("fork failed, " + std::string(::strerror(errno)));

	if ( child == 0 ) {

		this -> status._pid = ::getpid();

		if ( env_strings.empty() && this -> copy_env )
			ret = ::execvp(exec_args[0], const_cast<char* const*>(exec_args.data()));
		else
			ret = ::execvpe(exec_args[0], const_cast<char* const*>(exec_args.data()), const_cast<char* const*>(env_args.data()));

		if ( ret == -1 ) {

			this -> status._code = errno;

			if ( !this -> fork )
				throw std::runtime_error(std::string(::strerror(errno)));

			return errno;
		}

		this -> status._code = ret;
		return ret;

	} else if ( child > 0 ) {

		if ( this -> hooks.before )
			this -> hooks.before(*this);

		::waitpid(child, &ret, 0);
		this -> status._code = ret;
		if ( this -> hooks.after )
			this -> hooks.after(*this);
		return ret;
	}

	return ret;
}
