# AUTO
![AUTO banner](https://raw.githubusercontent.com/jakubriegel/AUTO/master/docs/banner.png)

AUTO is a project of the system of full autonomous electric cabs running in [Poznań](https://goo.gl/maps/JcykRLkoVo22) featuring integrated simulator of vehicles and sample front-end. It also has integrated generator of random orders for rides.

## About
![AUTO features](https://raw.githubusercontent.com/jakubriegel/AUTO/master/docs/features.png)


## Implementation
System is implemented 100% in C++ with additional frameworks: [Crow](https://github.com/ipkn/crow), [nlohmann::json](https://github.com/nlohmann/json), [cURL](https://github.com/curl/curl) and [cURLpp](https://github.com/jpbarrette/curlpp). Front-end is written is [TypeScript](https://github.com/Microsoft/TypeScript). Map service is provided by Google Maps API. 

![AUTO technologies](https://raw.githubusercontent.com/jakubriegel/AUTO/master/docs/technologies.png)

Core of the app is based on modular structure. Main app is an object of class **AUTO**, from which other modules expand. They are: 
* **server** - a Crow application serving AUTO web app and JSON API
* **simulator of cars and bases** - here all of cars and bases logic happens
* **requests genetator** - generates random orders for rides, so that it can be seen, how the system could work in real life

Each of them is implemented as a thread. **AUTO** is where main logic happens. It receives data from other modules, process it and send dispositions to the modules back.

External modules of the system are: 
* **config.json** - the app reads it each time it starts. This file stores all paremeters of the system, such as: cars number, bases locations, service area...
* **TypeScript web app** - here the logic of sample front-end is implemented
* **html template** and other **static files** for web app

### Diagram of how AUTO works:
![AUTO all_modules](https://raw.githubusercontent.com/jakubriegel/AUTO/master/docs/all_modules.png)

Majority of code is explained by in-line comments. Main algorithms used in the project are explained [here](https://raw.githubusercontent.com/jakubriegel/AUTO/master/docs/AUTO_Jakub_Riegel_formatka.pdf)[in Polish].

## Try it!
You can get hands on it here: [auto.jrie.eu](http://auto.jrie.eu/). 

AUTO web application is just a sample of how could user-side of such system look. It is working on desktop and mobile, but proper functionality is only available in Google Chrome.

Note that some system down time is possible.

## Build and run it yourself

Necessary dependencies are g++, cURL and cURLpp. The app also uses Crow and nlohmann::json, but they are provided in this repository.

AUTO was implemented using Visual Studio Code. This repository contains configuration files for it.

For making it possible to run AUTO you also need individual Google API key, with Diresction API, Places API and Maps JavaScript API enabled. When you grab it, just paste it into `config.json` file at `maps/apiKey`. Note that to make request to Google from your server you have to register your domain on Google Cloud. More information about the API can be found [here](https://cloud.google.com/maps-platform/).


### Compiling 
First you need to compile TypeScript. Type `tsc` in the Terminal. Afer it you are ready to start C++ compilation with: `g++ app.cpp includes/implementation/*.cpp -o AUTO -std=c++11 -lboost_system -lpthread -lcurlpp -lcurl `

### Running
Type: `sudo ./AUTO config.json`.

To run AUTO continuously I use `screen`. Simply type `screen`, then `sudo ./AUTO config.json`. 
To detach your console type `CTRL+A` followed by `D`. Return to it is possible with `screen -r`.


## Credits
This is a project for my CS studies at [Poznan University of Technology](https://www4.put.poznan.pl/en).

---
[<img src="https://yt3.ggpht.com/a-/AJLlDp0OnTj3ja34dx-_Z0-aAV9prQz2qJ1wxEKMEg=s900-mo-c-c0xffffffff-rj-k-no" width="100dp" />](https://www4.put.poznan.pl/en)
[<img src="http://www.cie.put.poznan.pl/images/nowelogo3eng.png" height="70dp"/>](http://www.cie.put.poznan.pl/index.php?lang=en)
