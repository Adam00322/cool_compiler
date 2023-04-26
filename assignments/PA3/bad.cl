(* no error *)
class A {
};

(* error:  b is not a type identifier *)
Class b inherits A {
};

(* error:  a is not a type identifier *)
Class C inherits a {
};

(* error:  keyword inherits is misspelled *)
Class D inherts A {
};

(* error:  closing brace is missing *)
Class E inherits A {
;

class F inherits A {
	test1() : Int {
		let i : int <- 1, j : string in i
	};
	test2() : int { 1 };
	test3() : Int {
		{
			a = Double + a;
			a * 2;
			Int;
		}
	};
	test4( : Int;
    test5() : Int {
        case 1 of
        esac
    };
    test5() : Int {
        case 1 of
            a : Int;
        esac
    };
    test6() : Int {
        new 111
    };
    test7() : Int {
    };
    Double : Int;
};



