class A {
    a : Int;

    b : Int <- 1;

    addb(): Int {
        b <- (let x:Int <- 1 in 2)+3
    };

    assigna(aa : Int) : Void {
        a <- aa
    };

    assigna(aa : Int, bb : Int) : Void {{
        aa < bb;
        aa <= not a;
        a = ~aa;
        a = isvoid aa;
        a = true;
        a = false;
        a = 123;
        a = "abc";
        let x : Int <- 1 in a <- x;
        (new B).set_var(x);
        a <- (aa - bb) * 2 / 3;
    }};

    test() : Void {{
        if a < b then
            out_string("a < b\n")
        else
            out_string("a >= b\n")
        fi;
        
        while a < b loop
            out_string("a < b\n")
        pool;
    }};

    class_type(var : A) : SELF_TYPE {
        case var of
            a : A => out_string("Class type is now A\n");
            b : B => out_string("Class type is now B\n");
            c : C => out_string("Class type is now C\n");
            d : D => out_string("Class type is now D\n");
            e : E => out_string("Class type is now E\n");
            o : Object => out_string("Oooops\n");
        esac
   };
};

Class BB__ inherits A {
};
