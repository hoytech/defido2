pragma solidity ^0.6.0;

import "./EllipticCurve.sol";
import "./opengsn/BaseRelayRecipient.sol";

contract Defido2 is EllipticCurve, BaseRelayRecipient {
    uint[2] pubKey;
    mapping(bytes32 => bool) seenMessages;

    constructor(uint[2] memory pubKey_) public {
        pubKey = pubKey_;
    }

    receive() external payable { }

    function invoke(address to, uint value, bytes memory payload, bytes memory auth, uint[2] memory sig) public {
        bytes32 messageHash = sha256(abi.encodePacked(auth, sha256(abi.encodePacked(to, value, payload))));

        require(!seenMessages[messageHash], "message already invoked");
        require(validateSignature(messageHash, sig, pubKey), "invalid signature");

        seenMessages[messageHash] = true;

        (bool success, bytes memory returndata) = to.call{ value: value }(payload);
        require(success, string(returndata));
    }
}
