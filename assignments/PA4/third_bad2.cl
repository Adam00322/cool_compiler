class Main inherits IO {
    main() : Int {
        1
    };
};

(* let binding error *)
class A {
    init() : A {
        {
            let t0 : String <- 3 in
            {
                t0 <- t0;
            };
            let t1 : Int <- 2 ,
            t2 : Bool in 
            {
                let t3 : Int in
                {
                    t3 <- t1;
                };
                t3;
            };
            t1;
        }
    };
};

(* assign type error *)
class B {
    t1 : Int <- 1;
    init() : String {
        {
            let t1 : String <- "111" in
            {
                t1 <- 222;
            };
            t1;
        }
    };
};

(* expression parameter type error *)
class C {
    t1 : Int <- 1;
    t2 : Bool <- true;
    init() : Int {
        t1 <- t1 + t2
    };
};

(* if expression type error(expect to be bool) *)
class D {
    t1 : Int <- 2;
    t2 : Int <- 3;
    init() : Int {
        {
            if (t1 * t2) then 
                (t1 <- t2) 
            else 
                (t2 <- t1) 
            fi;
        }
    };
};

(* if return type error *)
class Base0 {

};
class Base1 inherits Base0 {

};
class Basel1 inherits Base0 {

};
class Baser1 inherits Base0 {

};
class Baser2 inherits Baser1 {

};
class TestLCA {
    init() : Base1 {
        if (true) then
            (new Basel1)
        else
            (new Baser2)
        fi
    };
};

(* case branch error *)
class TestBanch {
    init() : Base0 {
        case (new Baser2) of
            c1 : Basel1 => (new Basel1);
            c2 : Base0 => (new Base0);
            c3 : Base1 => (new Base1);
            c4 : Basel1 => (new Basel1);
            c5 : Object => (new Baser1);
        esac
    };
};

(* loop expression type error(expect to be bool) *)
class TestLoop {
    t1 : Int <- 2;
    t2 : Int <- 3;
    init() : Int {
        while (t1 / t2) 
        loop
            {
                1;
            }
        pool
    };
};

(* polymorphic assign error *)
class TestPoly {
    init() : Base0 {
        {
            let t1 : Base0 , t2 : Base1 , t3 : Basel1, t4 : Baser1 , t5 : Baser2 in
            {
                t1 <- new Base1;
                t2 <- new Base0;
                t3 <- new Baser1;
                t4 <- new Baser2;
                t5 <- new Baser2;
            };
        }
    };
};

(* subclass use method of non-ancestor's method *)
class C1 {
    func() : Object {
        new Object
    };
};

class C2 inherits C1 {

};

class TestDispatch {
    init() : Object {
        {
            (1 + true).copy();
            (1 + true).init();
            (new C2)@C1.func();
            (new C2)@C1.func1();
            (new C2)@C3.func();
        }
    };
};