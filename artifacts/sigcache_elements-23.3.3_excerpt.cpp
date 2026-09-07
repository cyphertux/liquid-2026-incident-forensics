        CSHA256 hasher = m_salted_hasher_schnorr;
        hasher.Write(hash.begin(), 32).Write(pubkey.data(), pubkey.size()).Write(sig.data(), sig.size()).Finalize(entry.begin());
    }

    // ELEMENTS:
    void ComputeEntryRangeProof(uint256& entry, const std::vector<unsigned char>& proof, const std::vector<unsigned char>& commitment) {
        CSHA256 hasher = m_salted_hasher_range_proof;
        hasher.Write(proof.data(), proof.size()).Write(commitment.data(), commitment.size()).Finalize(entry.begin());
    }
    void ComputeEntrySurjectionProof(uint256& entry, const uint256 &hash, const std::vector<unsigned char>& proof, const std::vector<unsigned char>& commitment) {
        CSHA256 hasher = m_salted_hasher_surjection_proof;
        hasher.Write(hash.begin(), 32).Write(proof.data(), proof.size()).Write(commitment.data(), commitment.size()).Finalize(entry.begin());
    }

    bool
    Get(const uint256& entry, const bool erase)
    {
        std::shared_lock<std::shared_mutex> lock(cs_sigcache);
        return setValid.contains(entry, erase);
    }

    void Set(const uint256& entry)
    {
        std::unique_lock<std::shared_mutex> lock(cs_sigcache);
        setValid.insert(entry);
    }
    uint32_t setup_bytes(size_t n)
    {
        return setValid.setup_bytes(n);
    }
};

/* In previous versions of this code, signatureCache was a local static variable
 * in CachingTransactionSignatureChecker::VerifySignature.  We initialize
 * signatureCache outside of VerifySignature to avoid the atomic operation per
 * call overhead associated with local static variables even though
 * signatureCache could be made local to VerifySignature.
*/
static CSignatureCache signatureCache;

// ELEMENTS:
static CSignatureCache rangeProofCache;
static CSignatureCache surjectionProofCache;

} // namespace

// To be called once in AppInitMain/BasicTestingSetup to initialize the
// signatureCache.
void InitSignatureCache()
{
    // nMaxCacheSize is unsigned. If -maxsigcachesize is set to zero,
    // setup_bytes creates the minimum possible cache (2 elements).
    size_t nMaxCacheSize = std::min(std::max((int64_t)0, gArgs.GetIntArg("-maxsigcachesize", DEFAULT_MAX_SIG_CACHE_SIZE) / 4), MAX_MAX_SIG_CACHE_SIZE) * ((size_t) 1 << 20);
    size_t nElems = signatureCache.setup_bytes(nMaxCacheSize);
    LogPrintf("Using %zu MiB out of %zu/4 requested for signature cache, able to store %zu elements\n",
            (nElems*sizeof(uint256)) >>20, (nMaxCacheSize*2)>>20, nElems);
}

bool CachingTransactionSignatureChecker::VerifyECDSASignature(const std::vector<unsigned char>& vchSig, const CPubKey& pubkey, const uint256& sighash) const
{
    uint256 entry;
    signatureCache.ComputeEntryECDSA(entry, sighash, vchSig, pubkey);
    if (signatureCache.Get(entry, !store))
        return true;
    if (!TransactionSignatureChecker::VerifyECDSASignature(vchSig, pubkey, sighash))
        return false;
    if (store)
        signatureCache.Set(entry);
    return true;
}

bool CachingTransactionSignatureChecker::VerifySchnorrSignature(Span<const unsigned char> sig, const XOnlyPubKey& pubkey, const uint256& sighash) const
{
    uint256 entry;
    signatureCache.ComputeEntrySchnorr(entry, sighash, sig, pubkey);
    if (signatureCache.Get(entry, !store)) return true;
    if (!TransactionSignatureChecker::VerifySchnorrSignature(sig, pubkey, sighash)) return false;
    if (store) signatureCache.Set(entry);
    return true;
}

//
// ELEMENTS CACHES

// To be called once in AppInit2/TestingSetup to initialize the rangeproof cache
void InitRangeproofCache()
{
    // nMaxCacheSize is unsigned. If -maxsigcachesize is set to zero,
    // setup_bytes creates the minimum possible cache (2 elements).
    size_t nMaxCacheSize = std::min(std::max((int64_t)0, gArgs.GetIntArg("-maxsigcachesize", DEFAULT_MAX_SIG_CACHE_SIZE) / 4), MAX_MAX_SIG_CACHE_SIZE) * ((size_t) 1 << 20);
    size_t nElems = rangeProofCache.setup_bytes(nMaxCacheSize);
    LogPrintf("Using %zu MiB out of %zu/4 requested for rangeproof cache, able to store %zu elements\n",
            (nElems*sizeof(uint256)) >>20, nMaxCacheSize>>20, nElems);
}

// To be called once in AppInit2/TestingSetup to initialize the surjectionrproof cache
void InitSurjectionproofCache()
{
    // nMaxCacheSize is unsigned. If -maxsigcachesize is set to zero,
    // setup_bytes creates the minimum possible cache (2 elements).
    size_t nMaxCacheSize = std::min(std::max((int64_t)0, gArgs.GetIntArg("-maxsigcachesize", DEFAULT_MAX_SIG_CACHE_SIZE) / 4), MAX_MAX_SIG_CACHE_SIZE) * ((size_t) 1 << 20);
    size_t nElems = surjectionProofCache.setup_bytes(nMaxCacheSize);
    LogPrintf("Using %zu MiB out of %zu/4 requested for surjectionproof cache, able to store %zu elements\n",
            (nElems*sizeof(uint256)) >>20, nMaxCacheSize>>20, nElems);
}

bool CachingRangeProofChecker::VerifyRangeProof(const std::vector<unsigned char>& vchRangeProof, const std::vector<unsigned char>& vchValueCommitment, const std::vector<unsigned char>& vchAssetCommitment, const CScript& scriptPubKey, const secp256k1_context* secp256k1_ctx_verify_amounts) const
{
    uint256 entry;
    rangeProofCache.ComputeEntryRangeProof(entry, vchRangeProof, vchValueCommitment);

    if (rangeProofCache.Get(entry, !store)) {
        return true;
    }

    if (vchRangeProof.size() == 0) {
        return false;
    }

    uint64_t min_value, max_value;
    secp256k1_pedersen_commitment commit;
    if (secp256k1_pedersen_commitment_parse(secp256k1_ctx_verify_amounts, &commit, &vchValueCommitment[0]) != 1)
            return false;

    secp256k1_generator tag;
    if (secp256k1_generator_parse(secp256k1_ctx_verify_amounts, &tag, &vchAssetCommitment[0]) != 1)
        return false;

    if (!secp256k1_rangeproof_verify(secp256k1_ctx_verify_amounts, &min_value, &max_value, &commit, vchRangeProof.data(), vchRangeProof.size(), scriptPubKey.size() ? &scriptPubKey.front() : NULL, scriptPubKey.size(), &tag)) {
        return false;
    }
