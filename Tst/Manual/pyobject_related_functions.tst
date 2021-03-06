LIB "tst.lib"; tst_init();
  pyobject pystr = "Kublai Khan";

  // Additional functionality through attrib
  attrib(pystr, "__class__");    // should always be "<type 'str'>"...
  proc(attrib(pystr, "count"))("K");

  pystr."__class__";             // <- Short notations
  pystr.count("a");              // Even shorter (if attribute's name is valid and unused)

  python_run("def func(): return 17");
  attrib(func);
  attrib(func, "func_name");
  attrib(func, "func_name", "byAnyOtherName");
  attrib(func, "func_name");
tst_status(1);$
