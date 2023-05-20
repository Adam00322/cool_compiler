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

	is_empty() : Bool {
		if top.getValue().substr(0,1) = "$" then
			true
		else 
			false
		fi
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

class Queue {
	s_in : Stack <- (new Stack).init();
	s_out : Stack <- (new Stack).init();

	init() : Queue {
		{
			self;
		}
	};

	push(x : String) : Queue {
		{
			s_in.push(x);
			self;
		}
	};

	pop() : String {
		if s_out.is_empty() = false then
			s_out.pop()
		else 
			let ret : String in 
			{
				if s_in.is_empty() = false then
				{
					while s_in.is_empty() = false
					loop
						let temp : String in 
						{
							temp <- s_in.pop();
							s_out.push(temp);
						}
					pool;
					ret <- s_out.pop();
				}
				else 
					ret <- "$"
				fi;
				ret;
			}
		fi
	};

	front() : String {
		if s_out.is_empty() = false then
			s_out.top()
		else 
			let ret : String in 
			{
				if s_in.is_empty() = false then
				{
					while s_in.is_empty() = false
					loop
						let temp : String in 
						{
							temp <- s_in.pop();
							s_out.push(temp);
						}
					pool;
					ret <- s_out.top();
				}
				else 
					ret <- "$"
				fi;
				ret;
			}
		fi
	};

	is_empty() : Bool {
		if s_in.is_empty() = true then
			if s_out.is_empty() = true then
				true
			else 
				false
			fi
		else
			false
		fi
	};
};

class Sched {
	sched_queue : Queue <- (new Queue).init();
	current_proc : String;
	init() : SELF_TYPE {
		{
			current_proc <- "$";
			self;
		}
	};

	activate_proc(proc : String) : SELF_TYPE {
		{
			sched_queue.push(proc);
			self;
		}
	};

	sched() : SELF_TYPE {
		{
			let next_proc : String in
			{
				if sched_queue.is_empty() = true then
					current_proc <- current_proc
				else
					{
						next_proc <- sched_queue.pop();
						if current_proc = "$" then
							-- do_nothing
							1
						else
							sched_queue.push(current_proc)
						fi;
						current_proc <- next_proc;
					}
				fi;
			};
			self;
		}
	};

	get_current_proc() : String {
		if current_proc = "$" then
			"no proc now!\n"
		else
			current_proc
		fi
	};

};

class JumpSched inherits Sched {
	sched() : SELF_TYPE {
		{
			let next_proc : String in
			{
				if sched_queue.is_empty() = true then
					current_proc <- current_proc
				else
					{
						next_proc <- sched_queue.pop();
						if current_proc = "$" then
							-- do_nothing
							1
						else
							sched_queue.push(current_proc)
						fi;
						current_proc <- next_proc;
						next_proc <- sched_queue.pop();
						if current_proc = "$" then
							-- do_nothing
							1
						else
							sched_queue.push(current_proc)
						fi;
						current_proc <- next_proc;
					}
				fi;
			};
			self;
		}
	};
};

class Main inherits IO {
	main() : Object {
		let c : String , sched : Sched, jsched : JumpSched in
		{
			sched <- (new Sched).init();
			jsched <- (new JumpSched).init();
			out_string(">");
			c <- in_string();
			while (not c = "x") loop 
			{
				if c = "i" then
				{
					let temp : String in
					{
						temp <- in_string();
						jsched.activate_proc(temp);
					};
				} 
				else 
				{
					if c = "j" then
						jsched.sched()
					else
						jsched@Sched.sched()
					fi;
					out_string(jsched.get_current_proc());
					out_string("\n");
				}	
				fi;
				out_string(">");
				c <- in_string();
			} 
			pool;
		}
	};
};
