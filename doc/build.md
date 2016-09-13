Build lib & build sample                           {#enet_build}
========================

@tableofcontents

Download:                                          {#enet_build_download}
=========

enet use some tools to manage source and build it:

lutin (build-system):                              {#enet_build_download_lutin}
---------------------

```{.sh}
	pip install lutin --user
	# optionnal dependency of lutin (manage image changing size for application release)
	pip install pillow --user
```


dependency:                                        {#enet_build_download_dependency}
-----------

```{.sh}
	mkdir framework
	cd framework
	git clone https://github.com/atria-soft/etk.git
	git clone https://github.com/atria-soft/ememory.git
	git clone https://github.com/atria-soft/algue.git
	git clone https://github.com/generic-library/gtest-lutin.git --recursive
	git clone https://github.com/generic-library/z-lutin.git --recursive
	git clone https://github.com/generic-library/openssl-lutin.git --recursive
	cd ..
```

sources:                                           {#enet_build_download_sources}
--------

```{.sh}
	cd framework
	git clone https://github.com/atria-soft/enet.git
	cd ..
```

Build:                                             {#enet_build_build}
======


library:                                           {#enet_build_build_library}
--------

```{.sh}
	lutin -mdebug enet
```

Sample:                                            {#enet_build_build_sample}
-------

```{.sh}
	lutin -mdebug enet-server-websocket enet-client-websocket
```


Run sample:                                        {#enet_build_run_sample}
===========

```{.sh}
	lutin -mdebug enet-server-websocket?run
```

In an other bash:

```{.sh}
	lutin -mdebug enet-client-websocket?run
```

