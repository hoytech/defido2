"use strict";

const ethers = require('ethers');
const fs = require('fs');





let configFile = process.argv[2];
if (!configFile) throw('need configFile');
let cmd = process.argv[3];
if (!cmd) throw('need cmd');


let config = JSON.parse(fs.readFileSync(configFile, 'utf8'));

if (!config.privKey) throw("no privkey added");


if (cmd === 'deploy') {
    deploy();
} else if (cmd === 'senderAddr') {
    senderAddr();
} else if (cmd === 'ethBalance') {
    ethBalance();
} else if (cmd === 'ethDeposit') {
    ethDeposit();
} else if (cmd === 'erc20info') {
    erc20info();
} else if (cmd === 'invoke') {
    invoke();
} else {
    throw("unrecognized command: " + cmd);
}



async function deploy() {
    let pubKey = process.argv[4];
    let pubKeySplit = ['0x'+pubKey.slice(2, 66), '0x'+pubKey.slice(-64)];

    let wallet = new ethers.Wallet(config.privKey);
    let provider = new ethers.providers.JsonRpcProvider();
    wallet = wallet.connect(provider);

    let nonce = await provider.getTransactionCount(wallet.address);
    if (nonce !== 0) throw(`nonce for address ${wallet.address} was ${nonce}, not 0`);

    let contracts = loadContracts();

    let factory = new ethers.ContractFactory(contracts.defido2Abi, contracts.defido2Bin, wallet);
    let defido2Contract = await factory.deploy(pubKeySplit, { gasLimit: 6000000, });
    console.log(defido2Contract.address);
}



async function senderAddr() {
    let wallet = new ethers.Wallet(config.privKey);
    let provider = new ethers.providers.JsonRpcProvider();
    wallet = wallet.connect(provider);

    console.log(wallet.address);
}



async function ethBalance() {
    let wallet = new ethers.Wallet(config.privKey);
    let provider = new ethers.providers.JsonRpcProvider();
    wallet = wallet.connect(provider);

    let output = {};

    output.walletBalance = ethers.utils.formatEther(await provider.getBalance(config.walletAddr));
    output.senderBalance = ethers.utils.formatEther(await wallet.getBalance());

    console.log(JSON.stringify(output));
}



async function ethDeposit() {
    let value = ethers.utils.parseEther(process.argv[4]);

    let wallet = new ethers.Wallet(config.privKey);
    let provider = new ethers.providers.JsonRpcProvider();
    wallet = wallet.connect(provider);

    let txParams = {
        to: config.walletAddr,
        value,
    };

    let tx = await wallet.sendTransaction(txParams);
    console.error(`Sent tx: ${tx.hash}`);

    let result = await tx.wait();
    console.error(`Tx mined in block ${result.blockNumber} (${result.blockHash})`);
}



async function erc20info() {
    let tokenAddr = process.argv[4];
    let spender = process.argv[5];

    const erc20Abi = [
        'function totalSupply() view returns (uint)',
        'function balanceOf(address tokenOwner) view returns (uint)',
        'function allowance(address tokenOwner, address spender) view returns (uint)',
        'function transfer(address to, uint tokens) returns (bool)',
        'function approve(address spender, uint tokens) returns (bool)',
        'function transferFrom(address from, address to, uint tokens) returns (bool)',
    ];

    let provider = new ethers.providers.JsonRpcProvider();
    let contract = new ethers.Contract(tokenAddr, erc20Abi, provider);

    let output = {};

    output.balance = ethers.utils.formatEther(await contract.balanceOf(config.walletAddr));

    if (spender) output.allowance = ethers.utils.formatEther(await contract.allowance(config.walletAddr, spender));

    console.log(JSON.stringify(output));
}



async function invoke() {
    let to = process.argv[4];
    let value = process.argv[5];
    let payload = process.argv[6];
    let auth = process.argv[7];
    let sig1 = process.argv[8];
    let sig2 = process.argv[9];

    const walletAbi = [
        'function invoke(address to, uint value, bytes payload, bytes auth, uint[2] sig)',
    ];

    let wallet = new ethers.Wallet(config.privKey);
    let provider = new ethers.providers.JsonRpcProvider();
    wallet = wallet.connect(provider);

    let contract = new ethers.Contract(config.walletAddr, walletAbi, wallet);
    console.error(`Invoking method to=${to} value=${value} payload=${payload}`);

    let tx = await contract.invoke(to, value, payload, auth, [sig1, sig2], { gasLimit: 6000000, }); // FIXME: gasLimit because ganache takes too long to estimateGas
    console.error(`Sent tx: ${tx.hash}`);

    let result = await tx.wait();
    console.error(`Tx mined in block ${result.blockNumber} (${result.blockHash})`);

    console.log(result);
}




/////////

function loadContracts() {
    let contracts = {};

    let defido2Spec = require('./build/Defido2.json');
    contracts.defido2Abi = JSON.parse(defido2Spec.contracts['contracts/Defido2.sol:Defido2'].abi);
    contracts.defido2Bin = defido2Spec.contracts['contracts/Defido2.sol:Defido2'].bin;

    return contracts;
}

function computeContractAddress(addr) {
    // Assumes nonce of address is 0
    let hash = ethers.utils.keccak256(ethers.utils.concat(['0xd694', ethers.utils.padZeros(addr, 20), "0x80"]));
    return ethers.utils.getAddress('0x' + hash.substr(26));
}
