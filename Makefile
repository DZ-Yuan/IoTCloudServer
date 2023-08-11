TOPDIR := .
HEAD_DIR := $(TOPDIR)/include
SRC_DIR := $(TOPDIR)/src
SRC:= 
LIB:=-lpthread -lrt
TARGET:=Demo

include $(HEAD_DIR)/Makefile

define t
t1=10; \
echo $${t1}; 
echo "This is a shell command"
endef

define t2
	for i in `find ./src/ -name *.cpp`; \
	do\
		echo $${i}; \
	done
endef

define t3
	SRCS:=abc 
	RES:=`echo $$(SRCS)`
endef

define t4
	SRC:=$(shell for f in `find $(SRC_DIR) -name *.cpp`; do file="$${file}"" $${f}"; done; echo "$${file}")
endef

define t5
	SRC:=$(shell \
		for f in `find $(SRC_DIR) -name *.cpp`; \
		do\
			file="$${file}"" $${f}"; \
		done; \
		echo "$${file}")
endef

define t6_1
	$(error "Abort by manual!")
endef

define t6
	@if [ ! -e ./README.md ] ; then \
		echo "Hell"; \
	fi
endef


all:
	@for f in `find $(SRC_DIR) -name *.cpp`; \
	do\
		SRC="$${SRC}"" $${f}";\
	done; \
	echo $${SRC}; \
	$(CXX) -O0 -g $${SRC} -I $(HEAD_DIR) $(LIB) -o $(TARGET)

prepare:
	for f in `find ${SRC_DIR} -name *.cpp`; \
	do\
		SRC:=$(SRC)$${f};\
	done
	echo "src file: $(SRC)"

test:
	@echo ${TOPDIR}
	$(call t)
	RC="abc"
	@echo "$${RC}"


test2:
	$(call t2)
	echo $(CXX)

test3:
	$(info $(call t3))
	$(info $(eval $(call t3)))
	@echo $(SRCS)
	@echo $(RES)

test4:
	$(info $(call t4))
	$(eval $(call t4))
	echo $(SRC)
	
test5:
	$(info $(call t5))
	$(eval $(call t5))
	@echo $(SRC)

test6:
	@echo $(VERSION)

test7: test7_1
	$(call t6)

test7_1:
	@if [ ! -e ./README.md ]; then false; fi

seek: 
	@for file in `find ./src/ -name *.cpp`;\
	do\
		echo $${file};\
	done
		
clean:
	@-if [ -e $(TOPDIR)/$(TARGET) ]; \
	then \
		rm $(TOPDIR)/$(TARGET); \
	fi
	@echo "clean up"

.PHONY: clean
