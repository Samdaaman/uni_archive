const db = require('../../config/db');
const fs = require('mz/fs');
const passwords = require('../utils/passwords');

const photoDirectory = './storage/photos/';
const defaultPhotoDirectory = './storage/default/';

exports.resetDb = async function () {
    let promises = [];

    const sql = await fs.readFile('app/resources/create_database.sql', 'utf8');
    promises.push(db.getPool().query(sql));  // sync call to recreate DB

    const files = await fs.readdir(photoDirectory);
    for (const file of files) {
        if (file !== '.gitkeep') promises.push(fs.unlink(photoDirectory + file));  // sync call to delete photo
    }

    return Promise.all(promises);  // async wait for DB recreation and photos to be deleted
};

exports.loadData = async function () {
    await populateDefaultUsers();
    try {
        const sql = await fs.readFile('app/resources/resample_database.sql', 'utf8');
        await db.getPool().query(sql);
    } catch (err) {
        console.log(err.sql);
        throw err;
    }

    const defaultPhotos = await fs.readdir(defaultPhotoDirectory);
    const promises = defaultPhotos.map(file => fs.copyFile(defaultPhotoDirectory + file, photoDirectory + file));
    return Promise.all(promises);
};

/**
 * Populates the User table in the database with the given data. Must be done here instead of within the
 * `resample_database.sql` script because passwords must be hashed according to the particular implementation.
 * @returns {Promise<void>}
 */
async function populateDefaultUsers() {
    const createSQL = `INSERT INTO User (name, email, password, city, country, photo_filename)
                       VALUES ?`;
    let {properties, usersData} = require('../resources/default_users');

    // Shallow copy all the user arrays within the main data array
    // Ensures that the user arrays with hashed passwords won't persist across multiple calls to this function
    usersData = usersData.map(user => ([...user]));

    const passwordIndex = properties.indexOf('password');
    await Promise.all(usersData.map(user => changePasswordToHash(user, passwordIndex)));

    try {
        await db.getPool().query(createSQL, [usersData]);
    } catch (err) {
        console.log(err.sql);
        throw err;
    }
}

async function changePasswordToHash(user, passwordIndex) {
    // TODO you need to implement "passwords.hash()" yourself, then uncomment the line below.
    user[passwordIndex] = await passwords.hash(user[passwordIndex]);

    // It is recommended you use a reputable cryptology library to do the actual hashing/comparing for you...
}

exports.executeSql = async function (sql) {
    try {
        const [rows] = await db.getPool().query(sql);
        return rows;
    } catch (err) {
        console.log(err.sql);
        throw err;
    }
};
