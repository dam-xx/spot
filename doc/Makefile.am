DOXYGEN = doxygen

.PHONY: doc html

doc: spot

spot: Doxyfile
	$(DOXYGEN)

clean-local:
	rm -rf spot.html spot.latex
