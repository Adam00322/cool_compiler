(* cycle inherit *)
class Father inherits Grand {
	a : Int;
};

class Grand inherits Son {
	b : Int;
};

class Son inherits Father {
	c : Int;
};