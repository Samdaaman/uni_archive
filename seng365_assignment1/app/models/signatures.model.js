const db = require('../../config/db');

exports.getAllSignatures = async function() {
    const q = "SELECT * From Signature"
    const [rows, _] = await db.getPool().query(q);
    signaturesDict = {}
    rows.forEach(element => {
        const petitionId = element.petition_id;
        if (petitionId in signaturesDict)
        {
            signaturesDict[petitionId] ++;
        }
        else
        {
            signaturesDict[petitionId] = 1;
        }        
    });
    return signaturesDict;
};

exports.getSignaturesForPetition = async function(petitionId) {
    const q = 'SELECT * FROM Signature JOIN User WHERE Signature.signatory_id = User.user_id AND petition_id = ?';
    const [rows, _] = await db.getPool().query(q, [petitionId]);
    
    signatures = []
    rows.forEach(row => signatures.push({
        signatoryId: row.signatory_id,
        name: row.name,
        city: row.city,
        country: row.country,
        signedDate: row.signed_date
    }));
    signatures = signatures.sort((a, b) => new Date(a.signedDate) > new Date(b.signedDate) ? 1 : -1);
    return signatures;
};

exports.signPetition = async function(signatoryId, petitionId) {
    const q = 'INSERT INTO Signature (signatory_id, petition_id, signed_date) VALUES (?, ?, ?)';
    await db.getPool().query(q, [signatoryId, petitionId, new Date()]);
};

exports.unsignPetition = async function(signatoryId, petitionId) {
    const q = 'DELETE FROM Signature WHERE signatory_id = ? AND petition_id = ?';
    await db.getPool().query(q, [signatoryId, petitionId]);
};