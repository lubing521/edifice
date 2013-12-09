#makefile created by 16236914@qq.com on August 10 2011.
#Using GNU make 3.81, GCC 4.3.2
#The projct aims to supply assistance functions.

dirname := $(notdir $(CURDIR))
library_linkname := lib$(dirname).so
library = $(library_linkname).$(VERSION_NUM)
soname = lib$(dirname).so.$(firstword $(subst ., ,$(VERSION_NUM)))
sources := $(wildcard *.cpp)
objects := $(subst .cpp,.o,$(sources))
dependencies := $(subst .cpp,.d,$(sources))
include_dirs := ../public

TESTING_DIR := ../debug
VERSION_NUM := 1.0.0
CXXFLAGS += -fvisibility=hidden
CPPFLAGS += -I$(include_dirs) -fPIC -g
LINKFLAGS := -shared -Wl,-soname,$(soname) -L. -lpthread -lrt -lpentagon -lCrossPlatform 
RM := rm -rf
MV := mv

vpath %.cpp .
vpath %.h . $(include_dirs)

.PHONY : target test clean

target : $(library) $(TESTING_DIR)/server $(TESTING_DIR)/client
	@echo [OKAY]

# just for the purpose of testing ...
test :

ifneq "$(MAKECMDGOALS)" "clean"
  -include $(dependencies)
endif

#$(call make-depend,source-file,object-file,depend-file) 
# -MM option causes g++ to omit "system" headers from the prerequisites list. 
# -MF option specifies the dependecy filename.
define make-depend
  g++ -MM -MF $3 -MT $2 $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) $1
endef

$(library) : $(wildcard lib*)
$(library) : $(objects)
	g++ $(LINKFLAGS) -o $@ $^
	cp *so* $(TESTING_DIR)
	-cd $(TESTING_DIR) && ln -sf $@ $(library_linkname) 2> /dev/null || cp $@ $(library_linkname) && cp $@ $(library_linkname).1

$(TESTING_DIR)/server $(TESTING_DIR)/client : server.cc client.cc $(library)
	cp *.cc $(TESTING_DIR)
	cd $(TESTING_DIR) && g++ $(CPPFLAGS) server.cc -o server -Wl,-rpath,. -L. -lpentagon -lEIBCommNEO
	cd $(TESTING_DIR) && g++ $(CPPFLAGS) client.cc -o client -Wl,-rpath,. -L. -lpentagon -lEIBCommNEO
	/sbin/ldconfig -n .

%.o : %.cpp
	$(call make-depend, $<,$@,$(subst .o,.d,$@))
	$(COMPILE.C) $(OUTPUT_OPTION) $<

clean :
	$(RM) $(library) $(objects) $(dependencies)
