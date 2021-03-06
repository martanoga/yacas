
#include "yacas/yacas.h"
#include "yacas/mathcommands.h"
#include "yacas/standard.h"

#define OPERATOR(kind,prec,name) \
  kind##operators[hash.LookUp(#name)] = LispInFixOperator(prec);

DefaultYacasEnvironment::DefaultYacasEnvironment(std::ostream& os)
  : output(os),
    infixprinter(prefixoperators,
                 infixoperators,
                 postfixoperators,
                 bodiedoperators),
    iEnvironment(coreCommands,userFunctions,
                 globals,hash,output,infixprinter,
                 prefixoperators,infixoperators,
                 postfixoperators,bodiedoperators,
                 protected_symbols, &input),
    input(iEnvironment.iInputStatus)
{
    // Define the built-in functions by tying their string representation
    // to a kernel callable routine.
#define CORE_KERNEL_FUNCTION(iname,fname,nrargs,flags) iEnvironment.SetCommand(fname,iname,nrargs,flags);
#define CORE_KERNEL_FUNCTION_ALIAS(iname,fname,nrargs,flags) iEnvironment.SetCommand(fname,iname,nrargs,flags);
#include "yacas/corefunctions.h"
#undef CORE_KERNEL_FUNCTION
#undef CORE_KERNEL_FUNCTION_ALIAS
#undef OPERATOR
}


CYacas::CYacas(std::ostream& os):
    environment(os)
{
}

void CYacas::Evaluate(const std::string& aExpression)
{
    LispEnvironment& env = environment.getEnv();
    int stackTop = env.iStack.size();

    env.iErrorOutput.clear();
    env.iErrorOutput.str("");

    std::ostringstream iResultOutput;

    LispPtr result;

    try
     {
         LispPtr lispexpr;
//printf("Input: [%s]\n",aExpression);
         if (env.PrettyReader())
         {
            const LispString* prettyReader = env.PrettyReader();
            std::string full(aExpression);
            full.push_back(';');
            StringInput input(full,env.iInputStatus);
            LispLocalInput localInput(env, &input);
            LispPtr args(nullptr);
            InternalApplyString(env, lispexpr,
                               prettyReader,
                               args);
         }
         else
         {
           LispString full(aExpression);
           full.push_back(';');
           StringInput input(full,env.iInputStatus);
           env.iInputStatus.SetTo("CommandLine");
           LispTokenizer &tok = *env.iCurrentTokenizer;
           InfixParser parser(tok, input,
                              env,
                              env.PreFix(),
                              env.InFix(),
                              env.PostFix(),
                              env.Bodied());
           parser.Parse(lispexpr);
         }

         env.iEvalDepth=0;
         env.iEvaluator->ResetStack();
         env.iEvaluator->Eval(env, result, lispexpr);

         // If no error encountered, print result
         if (env.PrettyPrinter())
         {
             LispPtr nonresult;
             InternalApplyString(env, nonresult,
                                 env.PrettyPrinter(),
                                 result);
         }
         else
         {
             InfixPrinter infixprinter(env.PreFix(),
                                       env.InFix(),
                                       env.PostFix(),
                                       env.Bodied());

             infixprinter.Print(result, iResultOutput, env);
             iResultOutput.put(';');
         }
         const LispString* percent = env.HashTable().LookUp("%");
         env.UnProtect(percent);
         env.SetVariable(percent,result,true);
         env.Protect(percent);
     } catch (const LispError& error) {
        HandleError(error, env, env.iErrorOutput);
     }

     env.iStack.resize(stackTop);

     _result = iResultOutput.str();
     _error = env.iErrorOutput.str();
}
