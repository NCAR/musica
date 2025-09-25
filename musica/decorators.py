
def py_constructor(init):
    def wrap(cls):
        orig = cls.__init__
        def new_init(self, *a, **kw):
            orig(self)            # call the C++ default init
            init(self, *a, **kw)  # call the Python init
        cls.__init__ = new_init
        return cls
    return wrap
