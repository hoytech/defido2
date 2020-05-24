pragma solidity ^0.6.0;

import "./EllipticCurve.sol";
import "./opengsn/BaseRelayRecipient.sol";

contract Defido2 is EllipticCurve, BaseRelayRecipient {
    uint[2] pubKey;
    mapping(bytes32 => bool) seenMessages;

    constructor(uint[2] memory pubKey_) public {
        pubKey = pubKey_;
    }

    function invoke(address to, bytes memory data, bytes memory auth, uint[2] memory sig) public {
        bytes32 messageHash = keccak256(abi.encodePacked(auth, to, data));

        require(!seenMessages[messageHash], "message already invoked");
        require(validateSignature(messageHash, sig, pubKey), "invalid signature");

        seenMessages[messageHash] = true;

        (bool success,) = to.call(data);
        require(success, "tx failed");
    }
}
