# Note, the submodules are no longer used since they caused too much grief!

upstream := $(shell git remote | grep upstream)

update:
ifeq ($(upstream), upstream)
	git pull upstream master && git submodule update
endif
	git pull && git submodule update

pull:
	git pull && git submodule update

push:
	git push


ifeq ($(upstream), upstream)
recover:
# This is an inelegant solution but helps if things get screwed up
# with the submodules not updating.  I think it always pulls from the
# master branch of the submodules and is equivalent to git pull origin
# master in each submodule directory.
	git pull upstream master && git submodule update --remote
endif

