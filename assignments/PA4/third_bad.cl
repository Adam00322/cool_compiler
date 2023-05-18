class C {
	a : Int;
	b : Bool;
	init(x : Int, y : Bool) : C {
		{
			a <- x;
			b <- y;
			self;
		}
	};
};

(* no main method *)
class Main {
    not_main(): C {
		(* all method faults *)
        {
            (new C).init(1,1);
            (new C).init(1,true,3);
            (new C).iinit(1,true);
            (new C);
        }
	};
};

(* redefine method and attri in a class *)
(* bad attri name like self *)
class A {
    my_do1(): Object {
        self
    };
    my_do2(): IO {
        self
    };
    my_do1(): String {
        "--"
    };
    mem1: Bool;
    mem2: String;
    mem1: Object;
    self: SELF_TYPE;
};

(* bad override *)
class Shape {
    get_size() : Int {
        1
    };
    get_name() : String {
        "s"
    };
    get_func(t1 : Bool, t2 : Bool) : Int {
        3
    };
};

class Square inherits Shape {
    a : Int;
    get_size(a : Int) : Int {
        a*a
    };
    get_name() : Int {
        2
    };
    get_func(t1 : Bool, t2 : String) : Int {
        3
    };
};

