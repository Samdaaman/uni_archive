const crypto = require('crypto');

exports.hash = function (password) {
    if (password == null || password == "") {
        return "";
    } else {
        const hash = crypto.createHash('md5');
        hash.update(password);
        const hexStr = hash.digest('hex');
        // console.log(`Hashed ${password} to ${hexStr}`);
        return hexStr;
    }
}