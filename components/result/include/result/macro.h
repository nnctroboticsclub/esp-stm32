// Macros for unwrapping result
#define CAT_IMPL(s1, s2) s1##s2
#define CAT(s1, s2) CAT_IMPL(s1, s2)

// Run a task and forwarding the error code
#define _RUN_TASK_V(result, res) \
  auto res = result;             \
  if (res.IsErr()) {             \
    return res.Error();          \
  }

// Run a task and forwarding the error code, and assign the result to a variable
#define _RUN_TASK(result, var, res) \
  _RUN_TASK_V(result, res)          \
  auto var = res.Value();

// Run a task and forwarding the error code, and assign the result to a variable
#define _RUN_TASK_TO(result, var, res) \
  _RUN_TASK_V(result, res)             \
  var = res.Value();

// Get Unique variable name
#define UNIQUE_NAME(base) CAT(base, CAT(__LINE__, __COUNTER__))

// short hands for some common cases
#define RUN_TASK_V(result) _RUN_TASK_V(result, UNIQUE_NAME(res))
#define RUN_TASK(result, var) _RUN_TASK(result, var, UNIQUE_NAME(res))
#define RUN_TASK_TO(result, var) _RUN_TASK_TO(result, var, UNIQUE_NAME(res))