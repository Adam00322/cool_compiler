(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)

class Stack {
   top : Cons;
   
   init() : Stack {
      {
         top <- (new Cons).init("$", top);
         self;
      }
   };
   
   push(x : String) : Stack {
      {
         top <- (new Cons).init(x, top);
         self;
      }
   };
   
   top() : String { top.getValue() };
   
   pop() : String {
      let c : Cons <- top in {
         top <- c.getNext();
         c.getValue();
      }
   };
};

class Cons {
   head : String;
   tail : Cons;

   init(h:String,t:Cons) : Cons {
      { 
         head <- h;
         tail <- t;
         self;
      }
   };

   getValue() : String { head };
   
   getNext() : Cons { tail };
};

class Main inherits IO {
   main() : Object {
      let c : String,
      t : String,
      i : A2I <- new A2I,
      s2 : Stack <- (new Stack).init(),
      s : Stack <- (new Stack).init() in {
         out_string(">");
         c <- in_string();

         while (not c = "x") loop {
            if c = "e" then {
               t <- s.top();

               if t = "+" then { --两数相加
                  s.pop();
                  let a : Int <- i.a2i(s.pop()),
                  b : Int <- i.a2i(s.pop()) in {
                     s.push(i.i2a(a + b));
                  };
               } else if t = "s" then { --两项交换
                  s.pop();
                  let a : String <- s.pop(),
                  b : String <- s.pop() in {
                     s.push(a);
                     s.push(b);
                  };
               } else 0 fi fi;

            } else if c = "d" then { --输出栈内所有内容
               while (not s.top() = "$") loop {
                  out_string(s.top());
                  out_string("\n");
                  s2.push(s.pop());
               } pool;

               while (not s2.top() = "$") loop {
                  s.push(s2.pop());
               } pool;
            } else {
               s.push(c); --入栈
            } fi fi;

            out_string(">");
            c <- in_string(); --获取下一个指令
         } pool;

      }
   };
};
