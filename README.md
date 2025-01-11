[![License:MIT](https://img.shields.io/badge/License-MIT-blue?style=plastic)](LICENSE)
[![C++ CI build](../../actions/workflows/build.yml/badge.svg)](../../actions/workflows/build.yml)
### exec_cpp
C++ wrapper and utility around execvp and execvpe

This is a minimal library to assist executing from C++ program, it takes away
all the hassle with C char*'s and char**s by replacing them with vectors and maps.
Also configuring execution is made very simple while containing all simple requirements.

exec is initializable with config struct containing settings.

### <sub>configuration</sub>

exec has simple struct to use for configuration.
```
struct exec {

        public:

		...

                std::string cmd;
                std::vector<std::string> args;
                std::map<std::string, std::string> env;
                bool copy_env = true;
                bool fork = false;
                bool wait = true;
                functions hooks;

		...

                int perform();
};
```

You can initialize exec, like this:
```
exec e({ .cmd = "/bin/ls" });
```

or you can use assignment operator;
```
e = { .cmd = "/bin/ls", .args = "/" };
```

when ready to execute, ask it to perform:
```
e.perform();
```

You can retrieve execution return code via returned value, or ```e.status.code()```

### <sub>Example</sub>
See example for some use cases.
