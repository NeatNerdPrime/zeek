# @TEST-EXEC: btest-bg-run test1 zeek -b %INPUT
# @TEST-EXEC: btest-bg-wait 10
# @TEST-EXEC: cat test1/.stdout test1/.stderr >> out
# @TEST-EXEC: btest-diff out

redef exit_only_after_terminate = T;

event zeek_init()
{
	local h: addr = 127.0.0.1;

	when [h] ( local hname = lookup_addr(h) )
		{ 
		print "lookup successful";
		terminate();
		}
	timeout 10sec
		{
		print "timeout (1)";
		}

	local to = 5sec;
	# Just checking that timeouts can use arbitrary expressions...
	when [h] ( local hname2 = lookup_addr(h) ) {}
	timeout to {}
	when [h] ( local hname3 = lookup_addr(h) ) {}
	timeout to + 2sec {}

	# The following used to generate a spurious warning, so it's here
	# as a regression test.
	when ( local res = lookup_addr(127.0.0.1) )
		{
		return;
		}

	print "done";
}

