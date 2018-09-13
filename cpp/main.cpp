#include <iostream>
#include <functional>
#include <vector>

/* 

   Common stuff

 */
template <typename Context, typename Accumulator>
using DslFun = std::function<Accumulator(Context, Accumulator)>;

template <typename Context, typename Accumulator>
using DslBody = std::vector<DslFun<Context, Accumulator>>;

template <typename Context, typename Accumulator>
Accumulator runDsl(const Context& c, const Accumulator& acc0, 
  DslBody<Context, Accumulator> fs) {
  Accumulator acc = acc0;
  for (const auto& f: fs) {
    acc = f(c, acc);
  }
  return acc;
}

template <typename Context, typename Accumulator>
DslFun<Context, Accumulator> group(
  const DslBody<Context, Accumulator>& body) {
  return [=](const Context& c, const Accumulator& a) {
    return runDsl(c, a, body);
  };
}

/*

  Sample DSL for Java code generation

*/

struct JavaSrcContext {
  enum Visibility {
    Public, Private
  };
  
  Visibility visibility = Public;
  bool isStatic = false;
  std::string newLinePrefix = "\n";

  std::string staticStr() const {
    return isStatic? "static" : "";
  }

  std::string visibilityStr() const {
    switch (visibility) {
    case Public: return "public";
    case Private: return "private";
    };
  }
};

typedef DslFun<JavaSrcContext, std::string> JavaDslFun;
typedef std::vector<JavaDslFun> JavaBody;

JavaDslFun outputLine(bool newLine, std::vector<std::string> parts) {
  return JavaDslFun([=](JavaSrcContext c, std::string acc) {
      if (newLine) {
        acc += c.newLinePrefix;
      }
      bool rest = false;
      for (auto p: parts) {
        if (!p.empty()) {
          if (rest) {
            acc += " ";
          }
          acc += p;
          rest = true;
        }
      }
      return acc;
  });
}

JavaDslFun indentMore(JavaBody body) {
  return [=](JavaSrcContext c, std::string acc) {
    c.newLinePrefix += "  ";
    return runDsl(c, acc, body);
  };
}

JavaDslFun block(JavaBody body) {
  return group<JavaSrcContext, std::string>({
    outputLine(false, {" {"}),
    indentMore({group(body)}),
    outputLine(true, {"}"})
  });
}

JavaDslFun namedClass(const std::string& name, JavaBody body) {
  return [=](const JavaSrcContext& c, const std::string& acc) {
    return runDsl(c, acc, {
        outputLine(false, {c.visibilityStr(), "class", name}),
        block(body)
    });
  };
}

JavaDslFun Static(const JavaBody& body) {
  return [=](JavaSrcContext c, const std::string& acc) {
    c.isStatic = true;
    return runDsl(c, acc, body);
  };
}

JavaDslFun Private(const JavaBody& body) {
  return [=](JavaSrcContext c, const std::string& acc) {
    c.visibility = JavaSrcContext::Private;
    return runDsl(c, acc, body);
  };
}

JavaDslFun Variable(
  const std::string& type, const std::string& name) {
  return [=](const JavaSrcContext& c, const std::string& acc) {
    return runDsl(c, acc, {
        outputLine(true, {
            c.visibilityStr(), c.staticStr(), type, name, ";"})
      });
  };
}

int main() {
  auto code = namedClass("Kattskit", {
      Variable("double", "sum"),
      Variable("double", "sumSquares"),
      Variable("int", "count"),
      Private({
        Variable("boolean", "_isDirty")
      }),
      Static({
        Variable("int", "INSTANCE_COUNTER")
      })
  });

  std::cout << "The source code is\n" 
            << runDsl(JavaSrcContext(), 
                 std::string(""), {code}) << std::endl;
  return 0;
}
