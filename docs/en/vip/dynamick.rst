Dynamic Period K-lines
======================


.. py:function:: register_extra_ktype(ktype, basetype, nbars|minutes[, get_phase_end])
          
    Register an extended K-line type to implement custom dynamic period K-lines.

    Way 1: synthesize strictly based on the number of bars, e.g.: register_extra_ktype("DAY4, "DAY", 4)

    Way 2: synthesize based on time ranges; a conversion function that calculates the end time of the period from the current time needs to be defined, e.g. to define a 7-minute K-line:

    ::

      def get_min3_phase_end(datetime):
          m = datetime.minute
          if m % 3 == 0:
              return datetime
          m = (m / 3 + 1) * 3
          return Datetime(datetime.year, datetime.month, datetime.day, datetime.hour, m)

      register_extra_ktype("MIN3", "MIN", 3, get_min3_phase_end)

    .. note::
    
        Notes:
        
        1. Hikyuu has built in the DAY3, DAY5 and DAY7 extended K-lines based on the number of bars, and the MIN3 extended K-line based on time conversion
        2. It is recommended to create the custom conversion function that calculates the period end points of the K-line type in the way of a hub c++ part, because python has the GIL lock, and creating the conversion function in python may make it impossible to calculate with multiple threads
        3. Registering dynamic K-lines is not thread-safe; please do the other operations after the registration is completed
    
    :param str ktype: the extended K-line type name
    :param str basetype: the basic K-line type name
    :param int nbars: (or) minutes: the number of the basic periods or the minutes corresponding to the basic K-line type
    :param func get_phase_end: the date conversion function; the parameter is a date, and it returns the period end date corresponding to that date
    :return: None
