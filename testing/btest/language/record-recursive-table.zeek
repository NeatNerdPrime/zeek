# @TEST-EXEC: zeek -b %INPUT >output
# @TEST-EXEC: btest-diff output

type Foo: record {
	bar: count;
};

redef record Foo += {
	parent: table[bool] of Foo &default=table();
};

event zeek_init()
	{
	local tbl: table[Foo] of set[count];
	local f = Foo($bar=1);
	tbl[f] = set(1);
	print "===", |tbl|, tbl;
	print tbl;

	local parent_tbl: table[bool] of Foo = [
		[T] = Foo($bar=2),
	];
	f$parent = parent_tbl;
	tbl[f] = set(2);

	# This now has two entries in the table, because
	# after setting f$parent, that's a different key.
	print "===", |tbl|, tbl;

	# Mutate f and use it to delete the first entry again.
	f$parent = table();
	delete tbl[f];

	# This will be size 1
	print "===", |tbl|, tbl;
	}
