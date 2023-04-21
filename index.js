function NKB(path, threads, bufsize) {
    
    const functions = require(`./build/Release/addon.node`);

    functions.init_db(path.length, path, threads, BigInt(bufsize));

    function elem(ptr, element) {
        
        if(typeof element === 'object') ptr = element.ptr; else if(typeof element === 'string') {

            element = String.fromCharCode(Buffer.byteLength(element)) + element;
            
            ptr = functions.request(1, ptr, Buffer.byteLength(element), element);

            if(ptr === null) {
                
                console.log(`Can't find element "${element}"`);

                return null;

            }

            ptr = ptr.ptr;

        } else if(typeof element === 'number') {
            
            element = String(element);

            element = String.fromCharCode(Buffer.byteLength(element)) + element;

            ptr = functions.request(1, ptr, Buffer.byteLength(element), element);

            if(ptr === null) {
                
                console.log(`Can't find element "${element}"`);

                return null;

            }

            ptr = ptr.ptr;

        }
        
        this.elem = (element) => {

            element = new elem(ptr, element);
            
            if(!element.elem) return null; else return element;
    
        }

        this.find = (path) => find(ptr, path);
        this.get = (path) => get(ptr, path);
        this.count = () => count(ptr);
        this.all = () => all(ptr);
        this.set = (path, value) => set(ptr, path, value);
        this.add = (path, value) => add(ptr, path, value);
        this.append = (path, value) => append(ptr, path, value);
        this.create = (name, type, value, disable_logs) => create(ptr, name, type, value, disable_logs);

    }

    function find(ptr, path) {
        
        if(typeof path !== 'string') path = String(path);

        path = String.fromCharCode(Buffer.byteLength(path)) + path;
        
        let res = functions.request(1, ptr, Buffer.byteLength(path), path);

        return res;

    }

    function get(ptr, path) {

        if(typeof path !== 'string') path = String(path);

        path = String.fromCharCode(Buffer.byteLength(path)) + path;
        
        let res = functions.request(2, ptr, Buffer.byteLength(path), path);

        return res;
        
    }

    function count(ptr) {

        let res = functions.request(4, ptr, 0);

        return res;

    }

    function all(ptr) {

        let res = functions.request(5, ptr, 0);

        return res;

    }

    function set(ptr, path, value) {

        if(typeof path !== 'string') path = String(path);

        path = String.fromCharCode(Buffer.byteLength(path)) + path;

        if(typeof value === 'string') value = BigInt(value); else if(typeof value === 'number') value = BigInt(value); else if(value === true) value = 3; else if(value === false) value = 4;
        
        let res = functions.request(8, ptr, Buffer.byteLength(path), path, (typeof value === 'string' ? Buffer.byteLength(value) : 0), value);

        if(!disable_logs && res === null) console.log(`Error while setting "${value}" to "${path}"`);

        return res;

    }

    function add(ptr, path, value) {

        if(typeof path !== 'string') path = String(path);

        path = String.fromCharCode(Buffer.byteLength(path)) + path;

        if(typeof value === 'string') value = BigInt(value); else if(typeof value === 'number') value = BigInt(value);
        
        let res = functions.request(9, ptr, Buffer.byteLength(path), path, value);

        if(!disable_logs && res === null) console.log(`Error while adding "${value}" to "${path}"`);

        return res;

    }

    function append(ptr, path, value) {

        if(typeof path !== 'string') path = String(path);

        path = String.fromCharCode(Buffer.byteLength(path)) + path;

        if(typeof value !== 'string') value = String(value);
        
        let res = functions.request(10, ptr, Buffer.byteLength(path), path, Buffer.byteLength(value), value);

        if(!disable_logs && res === null) console.log(`Error while setting "${value}" to "${path}"`);

        return res;

    }

    function create(ptr, name, type, value, disable_logs) {

        if(typeof name !== 'string') name = String(name);
        
        if(type === 3 && !value) type = 4; else if(type === 2) {

            if(typeof value !== 'bigint') value = BigInt(value);

        } 
        
        let res = functions.request(11, ptr, 0, "", Buffer.byteLength(name), name, type, (type === 5 ? Buffer.byteLength(value) : 0), value);
        
        if(!disable_logs && res === null) console.log(`Error while creating "${name}"`);

        return res;

    }

    this.float = 1;
    this.int = 2;
    this.bool = 3;
    this.str = 5
    this.obj = 6;
    
    this.elem = (element) => {

        element = new elem(3n, element);
        
        if(element.elem) return element; else return null;

    }

    this.find = (path) => find(3n, path);
    this.get = (path) => get(3n, path);
    this.count = () => count(3n);
    this.all = () => all(3n);
    this.set = (path, value) => set(3n, path, value);
    this.add = (path, value) => add(3n, path, value);
    this.append = (path, value) => append(3n, path, value);
    this.create = (name, type, value, disable_logs) => create(3n, name, type, value, disable_logs);

}

module.exports = NKB;


/*

    find: 1
        ptr, path_len, path

    get: 2
        ptr, path_len, path

    get_many: 3
        ptr, path_len, path, vars_count, vars

    count: 4
        ptr, path_len, path

    all: 5
        ptr, path_len, path

    get_of_all: 6
        ptr, path_len, path, var_len, var

    get_many_of_all: 7
        ptr, path_len, path, vars_count, vars

    set: 8
        ptr, path_len, path, value_len, value

    add: 9
        ptr, path_len, path, value_len, value

    append: 10
        ptr, path_len, path, value_len, value

    create: 11
        ptr, path_len, path, name_len, name, type, value_len, value

    delete: 12
        ptr, path_len, path

*/