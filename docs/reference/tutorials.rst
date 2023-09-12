Tutorials 
#########

.. contents:: Table of Contents
    :depth: 3

Required Reading
================

There are many niche topics important to understand how SPICE and kernels work. The important things to know: 

1. What are kernels?
2. What are the different kernel types?
3. What are the different kernel qualities?

Here are some resources: 

* Intro to Kernels, includes descriptions of kernel types: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/Tutorials/pdf/individual_docs/12_intro_to_kernels
* Useful glossary of terms: https://isis.astrogeology.usgs.gov/documents/Glossary/Glossary.html
* Kerneldbgen, used to create ISIS DB files, docs has useful descriptions of Kernel qualities: https://isis.astrogeology.usgs.gov/Application/presentation/Tabbed/kerneldbgen/kerneldbgen.html
* NAIF required reading on SPICE: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/index.html
* ISIS repo, for more general information, many wikis: https://github.com/DOI-USGS/ISIS3/pulls?q=is%3Apr+is%3Aclosed

Creating New Mission Config Files
=================================

.. _kerneldbgen: https://isis.astrogeology.usgs.gov/Application/presentation/Tabbed/kerneldbgen/kerneldbgen.html
.. _emcascript: http://www.cplusplus.com/reference/regex/ECMAScript/

NAIF does not provide adequate resources to programmatically get all required metadata. ISIS solves this problem by creating "DB Files", PVL files containing metadata for every kernel in the ISIS data area. These DB files are a predominately human curiated in that either the Kernel files are hand written or a script (generally called `makedb`, co-located with the kernels in the kernel directory) is hand written to use kerneldbgen_ app to cache most of the kernel metadata. 

Issues with ISIS's approach: 

* DB files are co-located with kernels and therefore makes it difficult to release them seperately from the kernels.
* Kernels are specifically tied to the ISIS data area and cannot be used directly on NAIF distributed kernels. 
* As these DB files are in the ISIS data area, they cannot be versioned controlled seperately.
* If you add kernels, you have to regenerate the kernel files. 
* ISIS's code for searching and exploiting Kernels is not modularied. 
* Kernel search works on a per-image basis and cannot be used for arbitrary SPICE queries. 

SpiceQL more-or-less rewrites much of ISIS's Kernel searching and exploitation code.

Distictions in SpiceQL's approach: 

* DB files, here reffered to Mission config files, are located JSON files that can be shipped seperate from data installs.
* Config files leverage common Kernel naming schemes so you can use both NAIF installed kernels, but also kernels from USGS Astro or any other source. 
* Config files are source controlled.
* If you add Kernels, you can easily modify them locally, and contribute your changes for the new Kernels. 
* Sugar Spice modularizes most of the Kernel code for search and I/O. 
* Config files can be used for more generalized kernel queries

Understanding how ISIS stores Kernel metadata
*********************************************

Structure of ISIS data area, ignoring non-kernel data: 

.. code-block:: shell 
   
    .
    └── $ISISDATA/
        └── <mission>/
            └── kernels/
                ├── <Binary Kernel Type>/
                │   ├── <kernel db files> 
                │   ├── <Kernel makedb files> 
                │   └── <kernel files>
                └── <Text Kernel Type>/
                    ├── <kernel db files> 
                    └── <kernel files>


Where ``<mission>`` the name of any particular mission (mro, odyssey, messenger, etc.), ``<Kernel Type>`` is with slightly different structure if the kernel type is binary (CK, SPK, stc.) or text (IK, IAK, LSK, etc.), This section focuses on the Kernel "makedb" and "db" files. 

Each kernel directory (ck, spk, ik, etc.) have a file called ``kernels.[0-9]{4}.db`` where ``[0-9]{4}`` is the version of the db file. As ISIS kernel metadata is not version controlled, it uses a propriatary versioning system where the 4 numbers after the first extension delimiter is a version number.  

**SPK Kernel DB file, $ISISDATA/messenger/kernels/spk/kernels.0184.db**

.. code-block:: text 

    Object = SpacecraftPosition
    RunTime = 2015-09-14T13:41:46

    Group = Dependencies
        LeapsecondKernel = $base/kernels/lsk/naif0011.tls
    End_Group

    Group = Selection
        Time = ("2004 AUG 03 07:14:39.393205 TDB",
                "2015 APR 30 19:27:09.351488 TDB")
        File = $messenger/kernels/spk/msgr_20040803_20150430_od431sc_2.bsp
        Type = Reconstructed
    End_Group
    End_Object
    End

Some explinations: 

* ``Object = SpacecraftPosition`` specifiies this is SPK metadata. 
* ``Group = Dependencies`` specifies Kernels required in order to furnsh these kernels and extract meaningful data, in this case, an LSK. Usually, this is an LSK or SCLK. Sometimes includes Kernels for other reference frames. 
* ``Group = Selection`` specifies the criteria for selecting a particular kernel, in this case, the start and stop time range, the path of the Kernel and the Kernel quality. 

For binary kernels, these DB files are generally not hand written, instead the higher level information is written into a script, usually named ``makedb`` with information on required time kernels, and regexes for different kernel qualities and the kernel type. 

**Makedb script, ``$ISISDATA/messenger/kernels/spk/makedb``**

.. code-block:: bash

  #  Generate kernel database from existing kernels
  kerneldbgen to='kernels.????.db' type=SPK \
              recondir='$messenger/kernels/spk' \
              reconfilter=msgr_20040803*.bsp \
              lsk='$base/kernels/lsk/naif????.tls'

Things to note: 

* ``type=SPK`` describes the Kernel type, this populates the Object name in the output PVL. In this case, ``Object = SpacecraftPosition``  
* ``lsk='$base/kernels/lsk/naif????.tls`` describes the LSK dependencies, sometimes you also see ``SCLK=<path>`` in cases where a specific SCLK is required. Populates the dependencies group, in this case ``LeapsecondKernel = $base/kernels/lsk/naif0011.tls``
* ``reconfilter=msgr_20040803*.bsp`` describies the format (using Unix globbing wildcards) for this mission's reconstructed kernels for the given kernel type (SPK). Files matching these patterns are globbed, then: 
  1. kerneldb gen uses NAIFs cspice toolkit in order compute the time ranges for the kernels.
  2. Different filtereed as labeled. ``predictfilter`` matches are labeled as predicted, ``reconfilter`` matches are labeled as reconstructed, etc.   
  3. This process work very well as the names for mission kernels are very well structured. 

**Text Kernel file, $ISISDATA/mro/kernels/ik/kernels.0003.db**

.. code-block:: text 

    Object = Instrument
      Group = Selection
        Match = ("Instrument","InstrumentId","Marci")
        File  = ("mro", "kernels/ik/mro_marci_v??.ti")
      EndGroup
    
      Group = Selection
        Match = ("Instrument","InstrumentId","HIRISE")
        File  = ("mro", "kernels/ik/mro_hirise_v??.ti")
      EndGroup
    
      Group = Selection
        Match = ("Instrument","InstrumentId","CRISM")
        File  = ("mro", "kernels/ik/mro_crism_v??.ti")
      EndGroup
    EndObject

Here, like with most IK db files, the IK selection is instrument specific, so the ``Match`` key is used by ISIS to specify image label keywords that must match in order to select this kernel. These usually have no ``makledb`` and are hand written as scpice is not needed to extract kernel times. 

Last example, a more complex example where kernel selection requires multiple kernels selected from the same kernel type, this is used in situations where a particualr kernel has time-based dependencies. 

In this example, we use Messenger's MDIS. MDIS was mounted on a gimbal, so to get correct pointing information for MDIS relative to the Messenger spacecraft, the following frame chain is required: 

.. code-block:: text 
  
  Messenger Spacecraft -> Gimbal -> MDIS 

Therefore, we need one CK kernel for the spacecraft, the gimbal, and then for MDIS. ISIS accomplishes this by using 3 difference Kernel db files: ``messenger_kernels.[0-9]{4}.db``, ``messenger_mdis_att.[0-9]{4}.db`` and ``mdis_kernels.[0-9]{4}.db`` generated with seperate called to kerneldbgen. A hand written configuration file, called ``kernels.[0-9]{4}.conf``, is used to describe their relationships. 

**Conf file for Messenger, $ISISDATA/messenger/kernels/ck/kernels.0001.conf** 

.. code-block:: text 

  Object = Instrument
  
    Group = Selection
      Match = ("Instrument","InstrumentId","MDIS-WAC")
      File = ("messenger", "kernels/ck/mdis_kernels.????.db")
      File = ("messenger", "kernels/ck/messenger_mdis_att_kernels.????.db")
      File = ("messenger", "kernels/ck/messenger_kernels.????.db")
    End_Group
  
    Group = Selection
      Match = ("Instrument","InstrumentId","MDIS-NAC")
      File = ("messenger", "kernels/ck/mdis_kernels.????.db")
      File = ("messenger", "kernels/ck/messenger_mdis_att_kernels.????.db")
      File = ("messenger", "kernels/ck/messenger_kernels.????.db")
  End_Group
  
  End_Object
  End

**makedb file for Messenger, $ISISDATA/messenger/kernels/ck/kernels.0001.conf** 

.. code-block:: text 

  #  Generate kernel database from existing kernels

  kerneldbgen to='messenger_kernels.????.db' type=CK \
              recondir='$messenger/kernels/ck' \
              reconfilter='msgr_????_v??.bc' \
              smitheddir='$messenger/kernels/ck' \
              smithedfilter='msgr_mdis_sc????_usgs_v?.bc' \
              sclk='$messenger/kernels/sclk/messenger_????.tsc' \
              lsk='$base/kernels/lsk/naif????.tls'
              
  kerneldbgen to='messenger_mdis_att_kernels.????.db' type=CK \
              recondir='$messenger/kernels/ck' \
              reconfilter='msgr_mdis_sc??????_??????_sub_v?.bc' \
              sclk='$messenger/kernels/sclk/messenger_????.tsc' \
              lsk='$base/kernels/lsk/naif????.tls'
  
  kerneldbgen to='mdis_kernels.????.db' type=CK \
              recondir='$messenger/kernels/ck' \
              reconfilter='msgr_mdis_gm??????_??????v?.bc' \
              sclk='$messenger/kernels/sclk/messenger_????.tsc' \
              lsk='$base/kernels/lsk/naif????.tls'


Here, you can the how dependencies on other CK kernels are expressed. As MDIS-WAC and MDIS-NAC have the same dependencies, the two groups are the same. 

Everything here, except kernel start and stop times, require a knowledgable individual to manually label kernel qualities and dependencies. Times are cached by kerneldbgen_.

Translating .db/makedb files to a single configuration file 
************************************************************

.. warning:: As this library is a work in progress, much of this is subject to change. With so many missions with different requirements, it is very possible that this format misses supporting specific requirements for some missions. If you have any suggested changes for this proccess, feel free to open an issue and start a discussion. 

**Example Configuration File:**

Here, we'll run through an example config file, again for Messenger, encapsulating a lot of the information put into ISIS between the .db and makedb files.  

.. literalinclude:: ../../SpiceQL/db/mess.json 
   :language: JSON
   :linenos:


Things to note: 

* the JSON format mimics ISIS's directory structure for kernels.  With the tree: 

.. code-block:: shell 
   
    <mission>.json
     └── "<Frame>" 
          ├── "<Binary Kernel Type>:"
          │    └── "<Kernel Quality>:"
          │        └── <list of regexes>       
          ├── "<Text Kernel Type>:"
          │     └── <list of regexes> 
          └── "deps:"
               └── <list of json pointers> 

* Kernel formats now use EMCAScript_ (i.e. regular expressions).  
* No times are cached, these are simply computed at runtime.
* To resolve frame chains and dependencies on other JSON keywords, the deps object can be any arbitrary pointer to another JSON block which this frame depends on for SCLKs, CKs, etc. 
* Instead of a configs going into a black box which resolves per image kernels, running a query returns JSON matching this format. 

Example output for querying all kernels for Messenger: https://gist.github.com/Kelvinrr/3e383d62eead58c016b7165acdf64d60

The Flowchart™
**************

1. Identify the mission, create a new json file called <mission>.json, try to stick to NAIF abbreviations (see: https://naif.jpl.nasa.gov/pub/naif/pds/data/) 
2. Do binary kernels first (CKs, SPKs, etc.): 
    * Look at the makedb file, translate the wildcards to EMCAScript_ and place them under their respective quality keyword 
    * If list of regexes (or any list) has only one element, feel free to write it as a string, not an array
    * Check for a config file, make sure the dependencies on other time dependent kernels are handled with the `deps` keyword.  
3. When doing text kernels: 
    * Simply look at the `kernel.[0-9]{4}.db` file (specifically the newest one). Mimic the regexes into EMCAScript_ 
    * The key is to make sure the right text kernel is going to the right instrument. For example, IKs often are sometimes instrument specific, but also can be shared by all instruments on a spacecraft.  
4. Test by creating a new gtest running `utils::listMissionKernels` and manually confirming that all kernels are accounted for. Looks at FunctionalTestsSpiceQueries.cpp for some examples. 
5. Add new config files to CMakeLists.txt's SPICEQL_CONFIG_FILES variable to make sure they get installed. 
 
.. warning:: As this library is a work in progress, testing these queries isn't 100% figured out, the long term, we will test against ISIS's kernel search and make sure we get the response. 
