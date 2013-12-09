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

VERSION_NUM := 1.0.0
CXXFLAGS += -fvisibility=hidden
CPPFLAGS += -I$(include_dirs) -fPIC -g 
LINKFLAGS := -shared -Wl,-soname,$(soname) -L. -lpthread -lrt -lCrossPlatform 
RM := rm -rf
MV := mv

vpath %.cpp .
vpath %.h . $(include_dirs)

.PHONY : test clean
$(library) :

#test :
#echo $(OUTPUT_OPTION)

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
	cp $@ ../EIBCommNEO/
#下面的逻辑切换到EIBCommNEO目录建一个只有主版本号的软链接到pentagon库.但如果文件系统是win下的NTFS就会建链接失败。于是只好拷贝一份.
	-cd ../EIBCommNEO/ && ln -sf $(library) $(library_linkname) 2> /dev/null || cp $@ $(library_linkname) && cp $@ $(library_linkname).1
	cp $@ ../debug/
	-cd ../debug/ && ln -sf $(library) $(library_linkname).1 2> /dev/null || cp $(library) $(library_linkname).1

%.o : %.cpp
	$(call make-depend, $<,$@,$(subst .o,.d,$@))
	$(COMPILE.C) $(OUTPUT_OPTION) $<

clean :
	$(RM) $(library) $(objects) $(dependencies)
