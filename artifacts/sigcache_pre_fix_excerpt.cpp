}

// ELEMENTS:
void SignatureCache::ComputeEntryRangeProof(uint256& entry, const std::vector<unsigned char>& proof, const std::vector<unsigned char>& commitment) const {
    CSHA256 hasher = m_salted_hasher_range_proof;
    hasher.Write(proof.data(), proof.size()).Write(commitment.data(), commitment.size()).Finalize(entry.begin());
}
void SignatureCache::ComputeEntrySurjectionProof(uint256& entry, const uint256 &hash, const std::vector<unsigned char>& proof, const std::vector<unsigned char>& commitment) const {
    CSHA256 hasher = m_salted_hasher_surjection_proof;
    hasher.Write(hash.begin(), 32).Write(proof.data(), proof.size()).Write(commitment.data(), commitment.size()).Finalize(entry.begin());
}

bool SignatureCache::Get(const uint256& entry, const bool erase)
{
    std::shared_lock<std::shared_mutex> lock(cs_sigcache);
    return setValid.contains(entry, erase);
}

void SignatureCache::Set(const uint256& entry)
{
    std::unique_lock<std::shared_mutex> lock(cs_sigcache);
    setValid.insert(entry);
}

bool CachingTransactionSignatureChecker::VerifyECDSASignature(const std::vector<unsigned char>& vchSig, const CPubKey& pubkey, const uint256& sighash) const
{
    uint256 entry;
    m_signature_cache.ComputeEntryECDSA(entry, sighash, vchSig, pubkey);
    if (m_signature_cache.Get(entry, !store))
        return true;
    if (!TransactionSignatureChecker::VerifyECDSASignature(vchSig, pubkey, sighash))
        return false;
    if (store)
        m_signature_cache.Set(entry);
    return true;
}

bool CachingTransactionSignatureChecker::VerifySchnorrSignature(Span<const unsigned char> sig, const XOnlyPubKey& pubkey, const uint256& sighash) const
{
    uint256 entry;
    m_signature_cache.ComputeEntrySchnorr(entry, sighash, sig, pubkey);
    if (m_signature_cache.Get(entry, !store)) return true;
    if (!TransactionSignatureChecker::VerifySchnorrSignature(sig, pubkey, sighash)) return false;
    if (store) m_signature_cache.Set(entry);
    return true;
}

// TODO: de-globalise

// ELEMENTS CACHES
namespace {
    static SignatureCache rangeProofCache;
    static SignatureCache surjectionProofCache;
}
// To be called once in AppInit2/TestingSetup to initialize the rangeproof cache
bool InitRangeproofCache(size_t max_size_bytes)
{
    auto setup_results = rangeProofCache.setup_bytes(max_size_bytes);
    if (!setup_results) return false;
    const auto [num_elems, approx_size_bytes] = *setup_results;
    LogPrintf("Using %zu MiB out of %zu Mib requested for rangeproof cache, able to store %zu elements\n",
            approx_size_bytes >> 20, max_size_bytes >> 20, num_elems);
    return true;
}

// To be called once in AppInit2/TestingSetup to initialize the surjectionrproof cache
bool InitSurjectionproofCache(size_t max_size_bytes)
{
    auto setup_results = surjectionProofCache.setup_bytes(max_size_bytes);
    if (!setup_results) return false;
    const auto [num_elems, approx_size_bytes] = *setup_results;
    LogPrintf("Using %zu MiB out of %zu Mib requested for surjectionproof cache, able to store %zu elements\n",
            approx_size_bytes >> 20, max_size_bytes >> 20, num_elems);
    return true;
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

    if (!secp256k1_rangeproof_verify(secp256k1_ctx_verify_amounts, &min_value, &max_value, &commit, vchRangeProof.data(), vchRangeProof.size(), scriptPubKey.size() ? &scriptPubKey.front() : nullptr, scriptPubKey.size(), &tag)) {
        return false;
    }

    // An rangeproof is not valid if the output is spendable but the minimum number
    // is 0. This is to prevent people passing 0-value tokens around, or conjuring
    // reissuance tokens from nothing then attempting to reissue an asset.
    // ie reissuance doesn't require revealing value of reissuance output
    // Issuances proofs are always "unspendable" as they commit to an empty script.
    if (min_value == 0 && !scriptPubKey.IsUnspendable()) {
        return false;
    }

    if (store) {
        rangeProofCache.Set(entry);
    }

    return true;
