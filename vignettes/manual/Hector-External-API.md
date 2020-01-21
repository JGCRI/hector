# Hector External API

## Introduction

The Hector external API allows another program to execute one or more Hector runs from within another program. All of the functionality available in the stand-alone version of Hector is available through the API.  In fact, stand-alone Hector is implemented as a wrapper around a series of calls to the API.  Therefore, a program using the Hector API can do all of the following:  
* Set up one or more Hector instances and read configuration files.
* Set emissions values.
* Run Hector through a specified date, resuming from wherever the model last stopped.
* Retrieve current concentrations and forcings.

There are many ways these capabilities can be used, but our examples will concentrate on one work flow that will be interesting to integrated assessment (IA) model users:

1. Set up Hector.  
2. Run up to the start date of the IA model using stored historical emissions.  
3. Run an IA model step.  
9. Set Hector emissions from the IA model outputs.  
5. Run Hector to the start of the next IA model step.  
6. Read Hector outputs.  
7. Repeat steps 3-6 until the desired end date is reached.  

## Requirements

The Hector API is supported in Hector version 1.1 and later.  It is written in C++.  To use it, either your code must be written in C++, or you will need some way of calling C++ functions from whatever language you are using.  The instructions here presuppose a Linux install, but the same basic procedure, with some tweaking of file paths and so on, should work on Windows or Mac systems.

## Installation and Setup

[Install and build Hector|BuildHector] normally.  The build process will produce a library called `libhector.a`.  You will need to link this file into your executable when you build your program.  If you have a place where you store all of your libraries (e.g., `/usr/local/lib`), you can copy it there, or you can just use it in place if you wish.  You will also need to be able to include the Hector header files in your code.  They are found in the 'headers' directory under the directory where you installed hector.  You could copy that entire directory tree to something like `/usr/include/hector` if you wish, but it's probably just as easy to leave it where it is.

You don't absolutely need any of the other files in the Hector directories, but a few others might be useful as examples.  The file `source/main-api.cpp` is an example of how to use the API to get data in and out of Hector.  You can build the example code by uncommenting these lines in the Makefile:
```
# hector-api: libhector.a main-api.o
# 	$(CXX) $(LDFLAGS) -o hector-api main-api.o -lhector -lgsl -lgslcblas -lm
```
Then run `make hector-api` to build the example.  From the top-level hector directory, run `hector-api input/api-example.ini` to run the example code.  The example code reads emissions for several gasses from the files in the `input/emissions` directory and loads those emissions into the Hector core through the API.  Emissions for the remaining gasses are set from the input file, as in the stand-alone version of Hector.

## Using the API

Hector classes are (with one exception) contained in the `Hector::` namespace.  The classes needed to use the Hector API are:

* `Hector::Core`  Master controller that mediates interactions between Hector components and with outside code.  
* `Hector::unitval`  Structure for representing a numerical value with attached units.  Values that you send to and receive from Hector will be packaged in this structure.  The `unitval` class has an implicit conversion to double defined, which strips off the unit and returns the numerical value.  Writing a `unitval` to a `std::ostream` writes both the value and the unit (e.g. `12345 Tg`).
* `Hector::message_data`  Structure for holding messages for Hector components.  A message_data can hold a variety of different message types, but for messages from outside of Hector a message will be either a date (for retrieving data from a component), or a date paired with a `unitval` (for sending data to a component).
* `Hector::Logger`  Hector's logging class.  You only need to deal with this if you want to attach loggers to Hector components.
* `Hector::INIToCoreReader` Helper class used during initialization.  
* `Hector::CSVOutputStreamVisitor` Class that formats and writes Hector's output.  You will only need this if you want Hector to produce its native output.  If you are planning on retrieving data from Hector and using your code's output mechanism, you can skip it.
* `h_exception` Exception class.  If something goes wrong Hector will throw this class as an exception.

The Hector Core will be your interface for controlling and interacting with the simulation.  To run the simulation you use the Core's run method.  To get a component to do something you tell the Hector Core to send the component a message.  The type of the message determines what the component will do in response, and the contents of the message provide the input for whatever it is that the component will be doing.  The Core routes the message and returns the response, if any.  

A message comprises three parts:
* A **type** that indicates what you want the component to do.  The types you will encounter most frequently are *SETDATA* and *GETDATA*
* A **target** that indicates what the message is for.  Usually this will be the name of a gas or a type of forcing.  Valid targets have defined constants that are given in an [[appendix|Hector Message Targets]].
* The **contents** will provide the input for the component's action.  This will be a `message_data` object that you have packed with the relevant data.

### Initialization

The general procedure for initialization is:

1. Create an instance of `Hector::core`.
2. Initialize the core.
2. Read and parse the configuration (INI) file.
3. *(Optionally)* Attach an output stream visitor to Hector.
4. Set the core ready to run.  

All together, the procedure looks like this:

```C++
Hector::Core hcore;          // create the core
hcore.init();                // run init method

// create the INI reader:
Hector::INIToCoreReader coreParser(&hcore);
// use it to parse an input file:
coreParser.parse("input/myinput.ini");

/* This next part is OPTIONAL.  You only have to do it if you  want Hector's native output */
// set up an output file stream
std::ofstream ofile("output/my-hector-outputstream.csv");
// The output stream visitor formats and creates the output.  It needs
// the output stream we just created.
Hector::CSVOutputStreamVisitor hcosv(&ofile, true);
// Add the visitor to the core
hcore.addVisitor(&hcosv);
/* END of the OPTIONAL part */

hcore.prepareToRun();         // prepare Hector to run
```

### Setting Emissions

To set emissions for a gas, send a *SETDATA* message targeted at the gas you want to set emissions for.  You don't have to know which component handles the gas; the Core takes care of looking that up.  The message contents will be a date (the year you are setting emissions for) and a `unitval` (the emissions value).  Hector checks the units on the data passed in, but it doesn't convert on the fly.  [[Units for the gasses tracked by Hector|Hector Units]] are given in a separate appendix.

The entire process looks like this:
```C++
// emiss is a double with CO2 emissions in GtC/yr, presumably calculated elsewhere
// year is a double that gives the year
// hcore is an object of type Hector::Core
Hector::unitval data(emiss, U_PGC_YR);
Hector::message_data msg(year, data);
hcore.sendMessage(M_SETDATA, D_ANTHRO_EMISSIONS, msg);
```
You can ignore the return value from `sendMessage()` in this case, since the purpose of the message is to set data, not retrieve it.

Emissions set using this method overwrite any previously existing emissions for the same time and gas specified in the message.  This includes any emissions that were configured from the INI file.  As a result, it is safe to read a complete set of emissions from the INI file (e.g., by using one of the packaged configurations) and replace the emissions as needed.  Note, however, that the emissions data packaged with Hector are provided annually.  Therefore, if your model generates emissions at time steps of greater than one year, you will have to make certain to overwrite the emissions at years in between your model's time steps, or you will get unexpected results.

### Running the Model

To run the model, call the `Hector::Core::run` method:
```
hcore.run(stopYear);
```
The calculation will start at the start year configured in the INI file and will up to *and including* `stopYear`.  The practical upshot of this is that you should set the emissions for all years up to the desired stop year before calling `run()`.  

Subsequent calls will resume where the previous run left off and run up through the new stop year, so the following workflow is supported:
```C++
for(double year=model_start; year<=model_end; year+=model_timestep) {
  // run your model here...
  // set emissions from your model into Hector here...
  hcore.run(year);
  // get Hector results and do something with them here...
}
```
There is currently no way to make the model go backwards in time.  Attempting to run `Hector::run()` with an argument that is less than or equal to the last stop year will produce a warning but otherwise do nothing.  Running the model past the end date configured in the INI file will also produce a warning, but will run normally.  Note, however, that some of the components may use the configured end date in their calculations, and they are not guaranteed to be valid after the configured end date.  Therefore, it is safest to make sure that the end date configured in the INI file is on or after the latest date you expect to use in your run.

### Retrieving Data

To retrieve results from Hector, use a *GETDATA* message.  The procedure is similar to setting emissions data; however, in this case you will not need to supply a `unitval` argument, and you will want to keep the return value, since it contains the value you were retrieving.
```C++
// retrieve CO2 concentration at the current model time
Hector::unitval co2conc = hcore.sendMessage(M_GETDATA, D_ATMOSPHERIC_CO2);
// retrieve N2O concentration at specified year
Hector::unitval n2oconc = hcore.sendMessage(M_GETDATA, D_ATMOSPHERIC_N2O, year);
// retrieve total forcing at current model time
Hector::unitval rftot = hcore.sendMessage(M_GETDATA, D_RF_TOTAL);
```
There is a bit of inconsistency about the use of dates in these calls.  Generally, the date is forbidden for forcings, except for halocarbon forcings, where it is required.  Dates are forbidden for CO2 concentrations, but required for other gasses.  We hope to address these inconsistencies in subsequent updates.  Note that for outputs that do not allow a date to be supplied, the only way to get a value of the output for a particular date is to stop the run at that date and call `sendMessage()` to get the value for the current model time.

The return value is a `unitval` structure.  These can be manipulated directly, or if they are assigned to a double, the units will be stripped off leaving only the value:
```C++
double globaltemp = hcore.sendMessage(M_GETDATA, D_GLOBAL_TEMP);
double ocean_c_flux = hcore.sendMessage(M_GETDATA, D_OCEAN_CFLUX);
```

### Shutdown
To stop the core and delete all Hector components, call the `shutdown()` method:
```C++
hcore.shutdown();
```
Once this method is called, all further calls to methods on the Hector core will fail.  If you have instantiated more than one Hector Core, then only the core on which `shutdown()` is called will be affected.  Others will continue to operate normally.

### Exceptions
If Hector encounters an unrecoverable error, it will throw an exception with `h_exception` as the exception argument.  This object can be passed to an `ostream` to print the function, file, and line number where the problem was detected, along with a (hopefully) descriptive message.
```C++
try {
  // do some Hector calculations...
}
catch(const h_exception &e) {
  std::cerr << e << std::endl;  // prints info about what went wrong
  throw;
}
```

## Building and Running Your Code
There are two things you have to do to build a model linked with Hector.  The first is to make sure that the compiler can find the `hector.hpp` include file and all of the header files included from it.  You do this by adding the Hector headers directory to your compiler's include path.  For example, if you installed Hector in /usr/src/hector, then your compile command might look something like this:
```
g++ -I/usr/src/hector/headers -O -c mycode.cpp
```

The second thing you have to do is to link the object files and the Hector library, `libhector.a`.  You will also need to link in the GNU Scientific Library (GSL).  Similar to the compiling step, you have to set the library search path.  You also have to add the `-l` options of each of the needed libraries.  For example, with Hector in /usr/src/hector and the GSL libraries in /usr/local/lib, the link command looks like this:
```
g++ -L/usr/src/hector/source -L/usr/local/lib mycode.o -lhector -lgsl -lgslcblas -lm
```
The Hector library is statically linked, so you won't have to worry about library load paths at runtime.  The GSL is typically dynamically linked, so you will need to make sure your executable can find the library at runtime.  See the [[Hector build instructions|BuildHector]] for advice on how to arrange this.

If the build succeeds, then you should end up with an executable version of your model that you can run in your usual way.  Input will be under your code's control, so it will be up to you to supply the Hector libraries with the name of an INI file (see instructions above).  Unless you followed the instructions to create a Hector output stream, your code will also be responsible for producing whatever output you want from Hector.

## Limitations and Caveats
* If you're using a Hector output stream, and the code can't create the output file (e.g., because the directory doesn't exist), then the output will be silently discarded.  The same is true of the Hector log files.
* There is currently no way to rewind a Hector simulation to an earlier time.  If you want to do that, you have to shut down the Hector core and instantiate and initialize a whole new core.  The new core won't know anything about the emissions you put into the old core, so if you want it to follow the same emissions pathway up to the rewind point, your code will have to store those emissions and add them to the new core after it is initialized.  Then you can run Hector up to the point that you wanted to rewind to.