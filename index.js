function NKB(path, threads, bufsize) {
    
    const functions = require(`./addon.node`);

    functions.init_db(path.length, path, threads, BigInt(bufsize));

    function elem(ptr, element) {
        
        if(typeof element === 'object') {
        
            if(!element) {
                
                console.log(`Can't find element "${element}"`);

                return null;

            }
            
            ptr = element.ptr;
        
        } else if(typeof element === 'string') {

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
        this.get = (path, vars) => get(ptr, path, vars);
        this.count = () => count(ptr);
        this.all = () => all(ptr);
        this.getOfAll = (vars) => getOfAll(ptr, vars);
        this.set = (path, value) => set(ptr, path, value);
        this.add = (path, value) => add(ptr, path, value);
        this.append = (path, value) => append(ptr, path, value);
        this.create = (name, type, value, disable_logs) => create(ptr, name, type, value, disable_logs);
        this.remove = (name, disable_logs) => remove(ptr, name, disable_logs);

    }

    function find(ptr, path) {
        
        if(typeof path === 'object') {

            let new_path = "";

            for(let i = 0; i < path.length; i++) {
                
                if(typeof path[i] !== 'string') path[i] = String(path[i]);

                new_path += String.fromCharCode(Buffer.byteLength(path[i])) + path[i];

            }

            path = new_path;

        } else {
            
            if(typeof path !== 'string') path = String(path);

            path = String.fromCharCode(Buffer.byteLength(path)) + path;

        }
        
        let res = functions.request(1, ptr, Buffer.byteLength(path), path);

        return res;

    }

    function get(ptr, path, vars) {
        
        if(!vars) {
            
            if(typeof path === 'object') {

                vars = Array(path.length);

                for(let i = 0; i < path.length; i++) {

                    if(typeof path[i] !== 'string') path[i] = String(path[i]);

                    vars[i] = String.fromCharCode(Buffer.byteLength(path[i])) + path[i];
        
                    vars[i] = functions.request(2, ptr, Buffer.byteLength(vars[i]), vars[i]);

                }

            } else {
 
                if(typeof path !== 'string') path = String(path);
                
                vars = String.fromCharCode(Buffer.byteLength(path)) + path;
        
                vars = functions.request(2, ptr, Buffer.byteLength(vars), vars);
                
            }

        } else {
            
            if(typeof path === 'object') {

                let new_path = "";
    
                for(let i = 0; i < path.length; i++) {
                
                    if(typeof path[i] !== 'string') path[i] = String(path[i]);
    
                    new_path += String.fromCharCode(Buffer.byteLength(path[i])) + path[i];
    
                }
    
                path = new_path;
                
            } else {
                
                if(typeof path !== 'string') path = String(path);
    
                path = String.fromCharCode(Buffer.byteLength(path)) + path;
                
            }


            if(typeof vars === 'object') {

                for(let i = 0; i < vars.length; i++) {

                    if(typeof vars[i] !== 'string') vars[i] = String(vars[i]);

                    vars[i] = path + String.fromCharCode(Buffer.byteLength(vars[i])) + vars[i];
        
                    vars[i] = functions.request(2, ptr, Buffer.byteLength(vars[i]), vars[i]);

                }

            } else {
 
                if(typeof vars !== 'string') vars = String(vars);

                vars = path + String.fromCharCode(Buffer.byteLength(vars)) + vars;
        
                vars = functions.request(2, ptr, Buffer.byteLength(vars), vars);

            }

        }
 
        return vars;
        
    }

    function count(ptr) {

        let res = functions.request(4, ptr, 0);
        
        return res;

    }

    function all(ptr) {
        
        let res = functions.request(5, ptr, 0);
        
        return res;

    }

    function getOfAll(ptr, vars) {

        if(typeof vars === 'object') {

            let tmp;

            for(let i = 0; i < vars.length; i++) {

                if(typeof vars[i] !== 'string') vars[i] = String(vars[i]);
        
                vars[i] = functions.request(6, ptr, 0, "", Buffer.byteLength(vars[i]), vars[i]);

            }
            
            for(let i = 0; i < vars[0].length; i++) {

                tmp = vars[0][i].value;
                vars[0][i].value = Array(vars.length);
                vars[0][i].value[0] = tmp;

            }
            
            for(let i = 1; i < vars.length; i++) {

                for(let i2 = 0; i2 < vars[0].length; i2++) vars[0][i2].value[i] = vars[i][i2].value;

            }

            return vars[0];

        } else {
 
            if(typeof vars !== 'string') vars = String(vars);
        
            vars = functions.request(6, ptr, 0, "", Buffer.byteLength(vars), vars);

        }

        return vars;

    }

    function set(ptr, path, value, disable_logs) {
        
        if(typeof path === 'object') {

            let new_path = "";

            for(let i = 0; i < path.length; i++) {
                
                if(typeof path[i] !== 'string') path[i] = String(path[i]);

                new_path += String.fromCharCode(Buffer.byteLength(path[i])) + path[i];

            }

            path = new_path;

        } else {
            
            if(typeof path !== 'string') path = String(path);

            path = String.fromCharCode(Buffer.byteLength(path)) + path;

        }

        if(value === true) value = 3; else if(value === false) value = 4;
        
        let res = functions.request(8, ptr, Buffer.byteLength(path), path, value, (typeof value === 'string' ? Buffer.byteLength(value) : 0));

        if(!disable_logs && res === null) console.log(`Error while setting "${value}" to "${path}"`);

        return res;

    }

    function add(ptr, path, value, disable_logs) {

        if(typeof path === 'object') {

            let new_path = "";

            for(let i = 0; i < path.length; i++) {
                
                if(typeof path[i] !== 'string') path[i] = String(path[i]);

                new_path += String.fromCharCode(Buffer.byteLength(path[i])) + path[i];

            }

            path = new_path;

        } else {
            
            if(typeof path !== 'string') path = String(path);

            path = String.fromCharCode(Buffer.byteLength(path)) + path;

        }

        if(typeof value === 'string') value = Number(value);

        let old_value = functions.request(2, ptr, Buffer.byteLength(path), path);
        let res;

        if(typeof old_value === 'number' && (value += old_value) !== NaN) {
            
            res = functions.request(8, ptr, Buffer.byteLength(path), path, value);    

            if(!disable_logs && res === null) console.log(`Error while adding "${value}" to "${path}"`);

        } else console.log(`Error while adding "${value}" to "${path}"`);

        return res;

    }

    function append(ptr, path, value, disable_logs) {

        if(typeof path === 'object') {

            let new_path = "";

            for(let i = 0; i < path.length; i++) {
                
                if(typeof path[i] !== 'string') path[i] = String(path[i]);

                new_path += String.fromCharCode(Buffer.byteLength(path[i])) + path[i];

            }

            path = new_path;

        } else {
            
            if(typeof path !== 'string') path = String(path);

            path = String.fromCharCode(Buffer.byteLength(path)) + path;

        }

        if(typeof value !== 'string') value = String(value);

        let old_value = functions.request(2, ptr, Buffer.byteLength(path), path);
        let res;
        
        if(typeof old_value !== null) {

            old_value += value;
            
            res = functions.request(8, ptr, Buffer.byteLength(path), path, old_value, Buffer.byteLength(old_value));

            if(!disable_logs && res === null) console.log(`Error while appending "${value}" to "${path}"`);

        } else console.log(`Error while appending "${value}" to "${path}"`);

        return res;

    }

    function create(ptr, name, type, value, disable_logs) {

        if(typeof name !== 'string') name = String(name);
        
        if(type === 3 && !value) type = 4; else if(type === 5 && typeof value !== 'string') value = String(value);
        
        let res = functions.request(11, ptr, 0, "", Buffer.byteLength(name), name, type, (type === 5 ? Buffer.byteLength(value) : 0), value);
        
        if(!disable_logs && res === null) console.log(`Error while creating "${name}"`);

        return res;

    }

    function remove(ptr, path, disable_logs) {

        if(typeof path === 'object') {

            let new_path = "";

            for(let i = 0; i < path.length; i++) {
                
                if(typeof path[i] !== 'string') path[i] = String(path[i]);

                new_path += String.fromCharCode(Buffer.byteLength(path[i])) + path[i];

            }

            path = new_path;

        } else {
            
            if(typeof path !== 'string') path = String(path);

            path = String.fromCharCode(Buffer.byteLength(path)) + path;

        }
        
        let res = functions.request(12, ptr, Buffer.byteLength(path), path);
        
        if(!disable_logs && res === null) console.log(`Error while removing "${path}"`);

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
    this.get = (path, vars) => get(3n, path, vars);
    this.count = () => count(3n);
    this.all = () => all(3n);
    this.getOfAll = (vars) => getOfAll(3n, vars);
    this.set = (path, value) => set(3n, path, value);
    this.add = (path, value) => add(3n, path, value);
    this.append = (path, value) => append(3n, path, value);
    this.create = (name, type, value, disable_logs) => create(3n, name, type, value, disable_logs);
    this.remove = (name, disable_logs) => remove(3n, name, disable_logs);

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

    remove: 12
        ptr, path_len, path

*/
