// main class
class AUTO{
    public static: Static;

    private _cars: Data.Car[];
    get cars(): Data.Car[] { return this._cars; }

    private _bases: Data.Base[];
    get bases(): Data.Base[] { return this._bases; }

    private _port: Data.Port;
    get port(): Data.Port { return this._port; }
    set port(port: Data.Port) { this._port = port; }

    private map: google.maps.Map; 
    private markers: google.maps.Marker[];
    private area: Array<Data.Position>[];
    private areaPoly: google.maps.Polygon;

    private panel: HTMLDivElement;
    private requestForm: Module.RequestForm;
    private _overview: Module.Overview;
    get overview(): Module.Overview { return this._overview; }
    private _stats: Module.Stats;
    get stats(): Module.Stats { return this._stats; }

    constructor(){
        // initialize static data
        this.static = new Static();
        Data.Item.setStatic(this.static);

        // create loading screen
        let loading = <HTMLDialogElement> document.createElement('dialog');
        loading.id = "loading-screen";
        loading.appendChild(this.static.loading);
        let body = <HTMLBodyElement> document.getElementsByTagName('body')[0];
        body.appendChild(loading);
        loading.showModal();


        // initialize members
        this._cars = [];
        this._bases = [];
        this._port = null;
        this.panel = <HTMLDivElement> document.getElementById('panel');
 
        // get config data and initialize the map
        this.config();

        // initialize modules
        this.requestForm = new Module.RequestForm(this.panel, this);
        this._overview = new Module.Overview(this.panel);
        this._stats = new Module.Stats(this.panel);

        // get data and schedule updates 
        this.update();

        // remove loading screen
        loading.close()
        body.removeChild(loading);
        loading.removeChild(this.static.loading);
        
        // if page is displayed on small screen swap #map and #panel, so that #panel displays first
        window.onresize = () => { if(window.innerWidth < 766){
            let map = document.getElementById('map');
            body.removeChild(map);
            body.appendChild(map);}
        }
    }

    private config(): void {
        let app = this;
        // ask server for cars data
        let request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            // tasks to be done after response is received
            if (this.readyState == 4 && this.status == 200){
                let data = <Array<Data.Position>[]> JSON.parse(request.responseText);
                app.area = data;

                // initialize map | must be here because of asynchronuos requests
                app.initMap();
            }    
        }
        request.open("GET", "/auto/config", true); 
        request.send(null);
    }

    private initMap(): void {
        let poz = {lat: this.static.poznan.lat.valueOf(), lng: this.static.poznan.lng.valueOf()};
        this.map = new google.maps.Map(document.getElementById('map'), {
          zoom: 12,
          center: poz
        });
        
        this.areaPoly = new google.maps.Polygon({
            paths: this.area,
            strokeColor: '#000000',
            strokeOpacity: 0.9,
            strokeWeight: 1,
            fillColor: '#757575',
            fillOpacity: 0.1,
            clickable: false
        });
        this.areaPoly.setMap(this.map);
        
        this.markers = [];
    }

    private update(app: AUTO = this): void {
        // ask server for cars data
        let request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            // tasks to be done after response is received
            if (this.readyState == 4 && this.status == 200){
                let data = <Data.IUpdateData> JSON.parse(request.responseText);
        
                // update cars
                for(let d of data.cars){
                    (() => {
                        let dest; 
                        if(d.job.duringJob) dest = d.job.end;
                        else dest = "-";
                        for(let car of app.cars) if(d.id == car.getId()){
                            car.update(new Data.Position(d.pos.lat, d.pos.lng), d.status, d.battery, d.mileage, dest, d.warnings);
                            return;
                        }
                        app.cars.push(new Data.Car(d.id, d.status, new Data.Position(d.pos.lat, d.pos.lng), d.battery, d.mileage, dest, d.warnings, app.map));
                    })();
                }

                // update bases
                for(let d of data.bases){
                    (() => {
                        for(let base of app.bases) if(d.id == base.getId()){
                            base.update(d.free, d.cars.docked, d.cars.reserved);
                            return;
                        }
                        app.bases.push(new Data.Base(d.id, new Data.Position(d.pos.lat, d.pos.lng), app.map, d.name, d.slots, d.free, d.cars.docked, d.cars.reserved));
                    })();
                }

                // update port
                let port: any = data.port;
                if(app.port == null) app.port = new Data.Port(port.id, new Data.Position(port.pos.lat, port.pos.lng), app.map, port.name, port.slots, port.free, port.cars.docked, port.cars.reserved);
                else app.port.update(port.free, port.cars.docked, port.cars.reserved);

                // update modules
                app.overview.update(data.stats.active, data.stats.free, data.stats.busy, data.stats.preTime);
                app.stats.update(app.cars);
                
                // schedule next update
                setTimeout((_app = app)=>{ _app.update(_app); }, 15000);
            }
        }
        request.open("GET", "/auto/update", true); 
        request.send(null);
    }
}

// container for static data
class Static {
    public readonly poznan: Data.Position;

    public readonly autoFree: google.maps.Icon;
    public readonly autoTaken: google.maps.Icon;
    public readonly autoBusy: google.maps.Icon;
    public readonly base: google.maps.Icon;
    public readonly port: google.maps.Icon;

    public readonly loading: HTMLImageElement;

    constructor() {
        this.poznan = new Data.Position(52.403113, 16.925905);

        this.autoFree = {url: 'auto/static/auto_free.png', labelOrigin: new google.maps.Point(0, -15) };
        this.autoTaken = {url: 'auto/static/auto_taken.png', labelOrigin: new google.maps.Point(0, -15) };
        this.autoBusy = {url: 'auto/static/auto_busy.png', labelOrigin: new google.maps.Point(0, -15) };
        this.base = {url: 'auto/static/base.png', labelOrigin: new google.maps.Point(0, -15) };
        this.port = {url: 'auto/static/port.png', labelOrigin: new google.maps.Point(0, -15) };

        this.loading = document.createElement('img');
        this.loading.src = "/auto/static/loading.gif";
        this.loading.id = 'loading-bar';
    }
}

// start the app
window.onload = () => new AUTO();