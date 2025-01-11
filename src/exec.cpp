#include <stdexcept>
#include <algorithm>
#include <utility>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

#include "exec.hpp"

extern char **environ;

static std::vector<std::string> parse_environ() {

	if ( environ == nullptr ) return {};
	std::vector<std::string> ret;

	char** env = environ;
	for ( ;*env; ++env )
		ret.push_back(std::string(*env));

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
	std::vector<std::string> inherited_env;
	std::vector<const char*> env_args;

	std::transform(this -> env.begin(), this -> env.end(), std::back_inserter(env_strings),
			[](const std::pair<std::string, std::string>& p) { return !p.first.empty() && !p.second.empty() ? ( p.first + "=" + p.second ) : ""; });

	env_strings.erase(std::remove_if(env_strings.begin(), env_strings.end(),
					[](const std::string& s) { return s.empty(); }), env_strings.end());

	if ( this -> copy_env || !env_strings.empty()) {

		if ( this -> copy_env ) {

			inherited_env = parse_environ();
			std::transform(inherited_env.begin(), inherited_env.end(), std::back_inserter(env_args),
					[](const std::string& s) { return s.c_str(); });
		}

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
