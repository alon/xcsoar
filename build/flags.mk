CXX_FEATURES = -fno-exceptions 
C_FEATURES =

ALL_CPPFLAGS = $(TARGET_INCLUDES) $(INCLUDES) $(TARGET_CPPFLAGS) $(CPPFLAGS)
ALL_CXXFLAGS = $(OPTIMIZE) $(PROFILE) $(CXX_FEATURES) $(CXXFLAGS)
ALL_CFLAGS = $(OPTIMIZE) $(PROFILE) $(C_FEATURES) $(CFLAGS)
