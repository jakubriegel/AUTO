// main class
class AUTO{
    private static readonly poznan = new Data.Position(52.403113, 16.925905);
    private static: Static;

    private _cars: Data.Car[];
    get cars(): Data.Car[] { return this._cars; }

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
        this.static = new Static();
        Data.Car.setStatic(this.static);

        this._cars = [];
        this.panel = <HTMLDivElement> document.getElementById('panel');
 
        // get config data and initialize the map
        this.config();

        // initialize modules
        this.requestForm = new Module.RequestForm(this.panel);
        this._overview = new Module.Overview(this.panel);
        this._stats = new Module.Stats(this.panel);

        // get data and schedule updates 
        this.update();
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
        let poz = {lat: AUTO.poznan.lat.valueOf(), lng: AUTO.poznan.lng.valueOf()};
        this.map = new google.maps.Map(document.getElementById('map'), {
          zoom: 12,
          center: poz
        });
        
        this.areaPoly = new google.maps.Polygon({
            paths: this.area,
            strokeColor: '#000000',
            strokeOpacity: 0.8,
            strokeWeight: 1,
            fillColor: 'white',
            fillOpacity: 0.0,
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
                let data = <Data.ICarData> JSON.parse(request.responseText);
        
                // update cars
                for(let d of data.cars){
                    (() => {
                        for(let car of app.cars) if(d.id == car.getId()){
                            car.update(new Data.Position(d.pos.lat, d.pos.lng), d.status);
                            return;
                        }
                        app.cars.push(new Data.Car(d.id, d.status, new Data.Position(d.pos.lat, d.pos.lng), app.map, app.static));
                    })();
                    
                }

                // update modules
                app.overview.update(data.stats);
                app.stats.update(app.cars);
                
                // schedule next update
                setTimeout((_app = app)=>{ _app.update(_app); }, 15000);
            }
        }
        request.open("GET", "/auto/cars", true); 
        request.send(null);
    }
}

// container for static data
class Static {
    public readonly autoFree: google.maps.Icon;
    public readonly autoTaken: google.maps.Icon;
    public readonly autoBusy: google.maps.Icon;

    constructor() {
        this.autoFree = {url: 'auto/static/auto_free.png'};
        this.autoTaken = {url: 'auto/static/auto_taken.png'};
        this.autoBusy = {url: 'auto/static/auto_busy.png'};
    }
}

window.onload = () => {
    let auto = new AUTO();
};