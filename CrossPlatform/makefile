#makefile created by 16236914@qq.com on August 10 2011.
#Using GNU make 3.81, GCC 4.3.2
#The projct aims to supply assistance functions.

dirname := $(notdir $(CURDIR))
library = lib$(dirname).a
sources := $(wildcard *.cpp)
objects := $(subst .cpp,.o,$(sources))
dependencies := $(subst .cpp,.d,$(sources))
include_dirs := ../public

CXXFLAGS += 
CPPFLAGS += -I$(include_dirs) -g 
LINKFLAGS := 
RM := rm -rf
MV := mv

vpath %.cpp .
vpath %.h . $(include_dirs)

.PHONY : test clean

$(library) :

#just for temporary testing. 
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

$(library) : $(objects)
	ar rcs $@ $?	
	cp $@ ../pentagon/
	cp $@ ../EIBCommNEO/

%.o : %.cpp
	$(call make-depend, $<,$@,$(subst .o,.d,$@))
	$(COMPILE.C) $(OUTPUT_OPTION) $<

clean :
	$(RM) $(library) $(objects) $(dependencies)
