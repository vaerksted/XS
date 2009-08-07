/* eval.c -- evaluation of lists and trees ($Revision: 1.2 $) */

#include "es.hxx"

unsigned long evaldepth = 0, maxevaldepth = MAXmaxevaldepth;

static void failexec(Ref<const char> file, Ref<List> args) NORETURN;
static void failexec(Ref<const char> file, Ref<List> args) {
	List *fn;
	assert(gcisblocked());
	fn = varlookup("fn-%exec-failure", NULL);
	if (fn != NULL) {
		int olderror = errno;
		Ref<List> list = append(fn, mklist(mkstr(file.release()), args.release()));
		gcenable();
		eval(list.release(), NULL, 0);
		errno = olderror;
	}
	eprint("%s: %s\n", file.uget(), esstrerror(errno));
	exit(1);
}

/* forkexec -- fork (if necessary) and exec */
extern List *forkexec(const char *file, List *list, bool inchild) {
	gcdisable();
	Ref<Vector> env = mkenv();
	int pid = efork(!inchild, false);
	if (pid == 0) {
		execve(file, vectorize(list)->vector, env->vector);
		failexec(file, list);
	}
	gcenable();
	int status = ewaitfor(pid);
	if ((status & 0xff) == 0) {
		sigint_newline = false;
		SIGCHK();
		sigint_newline = true;
	} else
		SIGCHK();
	printstatus(0, status);
	return mklist(mkterm(mkstatus(status), NULL), NULL);
}

/* assign -- bind a list of values to a list of variables */
static List *assign(Ref<Tree> varform, Ref<Tree> valueform, Ref<Binding> binding) {
	Ref<List> result = NULL;

	Ref<List> vars = glom(varform.release(), binding.uget(), false);

	if (vars == NULL)
		fail("es:assign", "null variable name");

	Ref<List> values = glom(valueform.release(), binding.uget(), true);
	result = values;

	for (; vars != NULL; vars = vars->next) {
		Ref<List> value;
		Ref<const char> name = getstr(vars->term);
		if (values == NULL)
			value = NULL;
		else if (vars->next == NULL || values->next == NULL) {
			value = values;
			values = NULL;
		} else {
			value = mklist(values->term, NULL);
			values = values->next;
		}
		vardef(name.release(), binding.uget(), value.release());
	}

	return result.release();
}

/* letbindings -- create a new Binding containing let-bound (lexical) variables */
static Binding *letbindings(Ref<Tree> defn, Ref<Binding> binding,
			    Ref<Binding> context, int evalflags) {
	for (; defn != NULL; defn = defn->u[1].p) {
		assert(defn->kind == nList);
		if (defn->u[0].p == NULL)
			continue;

		Ref<Tree> assign = defn->u[0].p;
		assert(assign->kind == nAssign);
		Ref<List> vars = glom(assign->u[0].p, context.uget(), false);
		Ref<List> values = glom(assign->u[1].p, context.uget(), true);

		if (vars == NULL)
			fail("es:let", "null variable name");

		for (; vars != NULL; vars = vars->next) {
			Ref<List> value;
			Ref<const char> name = getstr(vars->term);
			if (values == NULL)
				value = NULL;
			else if (vars->next == NULL || values->next == NULL) {
				value = values;
				values = NULL;
			} else {
				value = mklist(values->term, NULL);
				values = values->next;
			}
			binding = mkbinding(name.release(), value.release(), binding.release());
		}
	}

	return binding.release();
}

/* localbind -- recursively convert a Bindings list into dynamic binding */
static List *localbind(Ref<Binding> dynamic, Ref<Binding> lexical,
		       Ref<Tree> body, int evalflags) {
	if (!dynamic)
		return walk(body.uget(), lexical.uget(), evalflags);
	else {
		Ref<List> result;
		Push p(dynamic->name, dynamic->defn);
		result = localbind(dynamic->next, lexical, body, evalflags);
		return result.release();
	}
}

/* local -- build, recursively, one layer of local assignment */
static List *local(Tree *defn, Ref<Tree> body,
		   Ref<Binding> bindings, int evalflags) {
	Ref<Binding> dynamic =
	    reversebindings(letbindings(defn, NULL, bindings, evalflags));
	return localbind(dynamic, bindings, body, evalflags);
}

/* forloop -- evaluate a for loop */
static List *forloop(Ref<Tree> defn, Ref<Tree> body,
		     Ref<Binding> outer, int evalflags) {
	static List MULTIPLE = { NULL, NULL };

	Ref<List> result = ltrue;
	Ref<Binding> looping = NULL;
	for (; defn != NULL; defn = defn->u[1].p) {
		assert(defn->kind == nList);
		if (defn->u[0].p == NULL)
			continue;
		Ref<Tree> assign = defn->u[0].p;
		assert(assign->kind == nAssign);
		Ref<List> vars = glom(assign->u[0].p, outer.uget(), false);
		Ref<List> list = glom(assign->u[1].p, outer.uget(), true);
		if (vars == NULL)
			fail("es:for", "null variable name");
		for (; vars != NULL; vars = vars->next) {
			Ref<const char> var = getstr(vars->term);
			looping = mkbinding(var.release(), list.uget(), looping.release());
			list = &MULTIPLE;
		}
		SIGCHK();
	}
	looping = reversebindings(looping.release());

	bool allnull;
	Ref<Binding> bp, lp, sequence; 
	Ref<List> value;

	try {
		for (;;) {
			allnull = true;
			bp = outer;
			lp = looping;
			sequence = NULL;
			for (; lp != NULL; lp = lp->next) {
				value = NULL;
				if (lp->defn != &MULTIPLE)
					sequence = lp;
				assert(sequence != NULL);
				if (sequence->defn != NULL) {
					value = mklist(sequence->defn->term,
						       NULL);
					sequence->defn = sequence->defn->next;
					allnull = false;
				}
				bp = mkbinding(lp->name, value.release(), bp.release());
			}
			if (allnull) {
				break;
			}
			result = walk(body.uget(), bp.uget(), evalflags & eval_exitonfalse);
			SIGCHK();
		}
	} catch (List *e) {
		if (!termeq(e->term, "break"))
			throwE(e);
		result = e->next;
	}
	return result.release();
}

/* matchpattern -- does the text match a pattern? */
static List *matchpattern(Ref<Tree> subjectform, Ref<Tree> patternform,
			  Ref<Binding> binding) {
	StrList *quote = NULL;
	Ref<List> subject = glom(subjectform.release(), binding.uget(), true);
	Ref<List> pattern = glom2(patternform.uget(), binding.uget(), &quote);
	return listmatch(subject.release(), pattern.release(), quote) 
		? ltrue 
		: lfalse;
}

/* extractpattern -- Like matchpattern, but returns matches */
static List *extractpattern(Ref<Tree> subjectform, Ref<Tree> patternform,
			    Ref<Binding> binding) {
	StrList *quote = NULL;
	Ref<List> result = NULL;
	Ref<List> subject = glom(subjectform.uget(), binding.uget(), true);
	Ref<List> pattern = glom2(patternform.uget(), binding.uget(), &quote);
	result = (List *) extractmatches(subject.release(), pattern.release(), quote);
	return result.release();
}

/* walk -- walk through a tree, evaluating nodes */
extern List *walk(Tree *tree0, Binding *binding0, int flags) {
	Tree *volatile tree = tree0;
	Binding *volatile binding = binding0;

	SIGCHK();

top:
	if (tree == NULL)
		return ltrue;

	switch (tree->kind) {

	    case nConcat: case nList: case nQword: case nVar: case nVarsub:
	    case nWord: case nThunk: case nLambda: case nCall: case nPrim: {
		Ref<Binding> bp = binding;
		Ref<List> list = glom(tree, bp.uget(), true);
		binding = bp.release();
		return eval(list, binding, flags);
	    }

	    case nAssign:
		return assign(tree->u[0].p, tree->u[1].p, binding);

	    case nLet: case nClosure: {
		Ref<Tree> body = tree->u[1].p;
		binding = letbindings(tree->u[0].p, binding, binding, flags);
		tree = body.release();
	    }
		goto top;

	    case nLocal:
		return local(tree->u[0].p, tree->u[1].p, binding, flags);

	    case nFor:
		return forloop(tree->u[0].p, tree->u[1].p, binding, flags);

	    case nMatch:
		return matchpattern(tree->u[0].p, tree->u[1].p, binding);

	    case nExtract:
		return extractpattern(tree->u[0].p, tree->u[1].p, binding);

	    default:
		panic("walk: bad node kind %d", tree->kind);

	}
	NOTREACHED;
}

/* bindargs -- bind an argument list to the parameters of a lambda */
extern Binding *bindargs(Ref<Tree> params, Ref<List> args, Ref<Binding> binding) {
	if (!params) return mkbinding("*", args.release(), binding.release());

	for (; params; params = params->u[1].p) {
		Ref<List> value;
		assert(params->kind == nList);
		Ref<Tree> param = params->u[0].p;
		assert(param->kind == nWord || param->kind == nQword);
		if (args == NULL)
			value = NULL;
		else if (params->u[1].p == NULL || args->next == NULL) {
			value = args;
			args = NULL;
		} else {
			value = mklist(args->term, NULL);
			args = args->next;
		}
		binding = mkbinding(param->u[0].s, value.release(), binding.release());
	}

	return binding.release();
}

/* pathsearch -- evaluate fn %pathsearch + some argument */
extern List *pathsearch(Term *term) {
	List *search, *list;
	search = varlookup("fn-%pathsearch", NULL);
	if (search == NULL)
		fail("es:pathsearch", "%E: fn %%pathsearch undefined", term);
	list = mklist(term, NULL);
	return eval(append(search, list), NULL, 0);
}

class Depth_tracker {
	public:
		Depth_tracker() {
			if (++evaldepth >= maxevaldepth)
				fail("es:eval", "max-eval-depth exceeded");
		}
		~Depth_tracker() {
			--evaldepth;
		}
};

/* eval -- evaluate a list, producing a list */
extern List *eval(Ref<List> list, Ref<Binding> binding, int flags) {
	Closure *volatile cp;

	Depth_tracker t;

	Ref<const char> name;

	Ref<const char> funcname = NULL;
	Ref<List> fn;

restart:
	if (list == NULL) return ltrue;
	assert(list->term != NULL);

	if ((cp = getclosure(list->term)) != NULL) {
		switch (cp->tree->kind) {
		    case nPrim:
			assert(cp->binding == NULL);
			list = prim(cp->tree->u[0].s, list->next, binding.uget(), flags);
			break;
		    case nThunk:
			list = walk(cp->tree->u[0].p, cp->binding, flags);
			break;
		    case nLambda:
		    {
			Ref<Tree> tree = cp->tree;
			Ref<Binding> context;
			      
			try {
				context =  bindargs(tree->u[0].p,
						    list->next,
						     cp->binding);
				if (funcname) {
					Push p("0",
					     mklist(mkterm(funcname.uget(),
							  NULL),
					 	    NULL));
					list = walk(tree->u[1].p, context.uget(), flags);
				} else list = walk(tree->u[1].p, context.uget(), flags);
			} catch (List *e) {
				if (termeq(e->term, "return")) {
					list = e->next;
					goto done;
				}
				throwE(e);
			}	
		    }
		    break;
		    case nList: {
			list = glom(cp->tree, cp->binding, true);
			list = append(list, list->next);
			goto restart;
		    }
		    default:
			panic("eval: bad closure node kind %d",
			      cp->tree->kind);
		    }
		goto done;
	}

	/* the logic here is duplicated in $&whatis */

	name = getstr(list->term);
	fn = varlookup2("fn-", name.uget(), binding.uget());
	if (fn != NULL) {
		funcname = name;
		list = append(fn, list->next);
		goto restart;
	}
	if (isabsolute(name.uget())) {
		const char *error = checkexecutable(name.uget());
		if (error)
			fail("$&whatis", "%s: %s", name.uget(), error);
		list = forkexec(name.uget(), list.release(), flags & eval_inchild);
		goto done;
	}

	fn = pathsearch(list->term);
	if (fn != NULL && fn->next == NULL
	    && (cp = getclosure(fn->term)) == NULL) {
		const char *name = getstr(fn->term);
		list = forkexec(name, list.release(), flags & eval_inchild);
		goto done;
	}

	list = append(fn, list->next);
	goto restart;

done:
	if ((flags & eval_exitonfalse) && !istrue(list.uget()))
		exit(exitstatus(list.uget()));
	return list.release();
}

/* eval1 -- evaluate a term, producing a list */
extern List *eval1(Term *term, int flags) {
	return eval(mklist(term, NULL), NULL, flags);
}
