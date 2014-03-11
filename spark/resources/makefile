all: resources.bin resources.h

RESOURCES = \
	chime.bin \
	siren.bin \
	wilhelm.bin

resources.bin: $(RESOURCES)
	cat $^ > $@

resources.h: $(RESOURCES) make_header.py make_sample.py
	python3 make_header.py $@ $(RESOURCES)

%.bin : %.ogg make_sample.py
	python3 make_sample.py $< $@

.PHONY: all
