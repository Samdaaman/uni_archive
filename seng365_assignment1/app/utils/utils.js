exports.parseText = function(text) {
    // Returns an empty string if the string is undefined or null
    if (typeof text == 'undefined')
    {
        return '';
    }
    else if (text == null) {
        return '';
    }
    else {
        return text.toString();
    }
}

exports.parseTextArray = function(arrayToParse) {
    let newArray = [];
    for (let i = 0; i < arrayToParse.length; i++) {
        newArray.push(exports.parseText(arrayToParse[i]))
    }
    return newArray;
}