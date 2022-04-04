const db = require('../../config/db');

function formatCategoryFromDB(row) {
    return {
        categoryId: row.category_id,
        name: row.name
    };
}

exports.getAllCategorys = async function() {
    const q = 'SELECT * FROM Category';
    const [rows, _] = await db.getPool().query(q);
    
    let categories = []
    rows.forEach(row => categories.push(formatCategoryFromDB(row)));
    return categories;
};

exports.getCategoryName = function(categoryId, categories) {
    let category = categories.find(category => category.categoryId === categoryId)
    if (category !== undefined) {
        return category.name;
    } else {
        return null;
    }
};

exports.getCategoryId = function(categoryName, categories) {
    let category = categories.find(category => category.name === categoryName)
    if (category !== undefined) {
        return category.categoryId;
    } else {
        return -1;
    }
};

