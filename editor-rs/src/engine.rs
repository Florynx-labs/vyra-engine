//! Safe Rust ownership and error-handling layer for the VYRA editor C ABI.
use std::{ffi::{CStr, CString, NulError, c_char}, marker::PhantomData, ptr::NonNull};

const ENTITY_NAME_CAPACITY: usize = 128;
const EXPECTED_ABI_VERSION: u32 = 1;

#[repr(C)]
struct RawEngine { _private: [u8; 0] }

#[repr(C)]
struct RawEntityInfo { id: u64, name: [c_char; ENTITY_NAME_CAPACITY] }

#[link(name = "vyra_editor_bridge")]
unsafe extern "C" {
    fn vyra_editor_bridge_abi_version() -> u32;
    fn vyra_editor_engine_create() -> *mut RawEngine;
    fn vyra_editor_engine_destroy(engine: *mut RawEngine);
    fn vyra_editor_engine_last_error(engine: *const RawEngine) -> *const c_char;
    fn vyra_editor_engine_entity_count(engine: *mut RawEngine) -> usize;
    fn vyra_editor_engine_entity_at(engine: *mut RawEngine, index: usize, out_entity: *mut RawEntityInfo) -> i32;
    fn vyra_editor_engine_create_entity(engine: *mut RawEngine, name: *const c_char, out_entity_id: *mut u64) -> i32;
    fn vyra_editor_engine_rename_entity(engine: *mut RawEngine, entity_id: u64, name: *const c_char) -> i32;
    fn vyra_editor_engine_destroy_entity(engine: *mut RawEngine, entity_id: u64) -> i32;
    fn vyra_editor_engine_clear_scene(engine: *mut RawEngine) -> i32;
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EntitySnapshot { pub id: u64, pub name: String }

#[derive(Debug)]
pub enum EngineError {
    LibraryUnavailable,
    AbiVersionMismatch { expected: u32, actual: u32 },
    InvalidName(NulError),
    Native(String),
    InvalidUtf8,
}

impl std::fmt::Display for EngineError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::LibraryUnavailable => write!(f, "VYRA editor bridge could not create an engine instance"),
            Self::AbiVersionMismatch { expected, actual } => write!(f, "VYRA editor ABI mismatch: expected {expected}, received {actual}"),
            Self::InvalidName(error) => write!(f, "Entity name contains an interior NUL byte: {error}"),
            Self::Native(message) => write!(f, "VYRA engine error: {message}"),
            Self::InvalidUtf8 => write!(f, "VYRA engine returned a name that is not valid UTF-8"),
        }
    }
}
impl std::error::Error for EngineError {}

/// Owns one native scene. The handle is destroyed exactly once by Drop.
/// The C++ scene is not thread-safe, so this wrapper is neither Send nor Sync.
pub struct EngineBridge { raw: NonNull<RawEngine>, _not_send_or_sync: PhantomData<*mut ()> }

impl EngineBridge {
    pub fn new() -> Result<Self, EngineError> {
        // SAFETY: The ABI version function has no arguments or side effects.
        let actual = unsafe { vyra_editor_bridge_abi_version() };
        if actual != EXPECTED_ABI_VERSION {
            return Err(EngineError::AbiVersionMismatch { expected: EXPECTED_ABI_VERSION, actual });
        }
        // SAFETY: The bridge returns a newly allocated opaque handle.
        let raw = NonNull::new(unsafe { vyra_editor_engine_create() }).ok_or(EngineError::LibraryUnavailable)?;
        Ok(Self { raw, _not_send_or_sync: PhantomData })
    }

    pub fn entities(&mut self) -> Result<Vec<EntitySnapshot>, EngineError> {
        // SAFETY: self owns a live handle for the duration of this call.
        let count = unsafe { vyra_editor_engine_entity_count(self.raw.as_ptr()) };
        let mut entities = Vec::with_capacity(count);
        for index in 0..count {
            let mut item = RawEntityInfo { id: 0, name: [0; ENTITY_NAME_CAPACITY] };
            // SAFETY: item is a valid writable C-layout output structure.
            let status = unsafe { vyra_editor_engine_entity_at(self.raw.as_ptr(), index, &mut item) };
            match status {
                1 => {
                    // SAFETY: The bridge writes a NUL-terminated name into the fixed buffer.
                    let name = unsafe { CStr::from_ptr(item.name.as_ptr()) }.to_str()
                        .map_err(|_| EngineError::InvalidUtf8)?.to_owned();
                    entities.push(EntitySnapshot { id: item.id, name });
                }
                0 => break,
                _ => return Err(self.last_error()),
            }
        }
        Ok(entities)
    }

    pub fn create_entity(&mut self, name: &str) -> Result<u64, EngineError> {
        let name = CString::new(name).map_err(EngineError::InvalidName)?;
        let mut id = 0;
        // SAFETY: CString is NUL-terminated and id is a valid output pointer.
        let status = unsafe { vyra_editor_engine_create_entity(self.raw.as_ptr(), name.as_ptr(), &mut id) };
        self.check_status(status)?;
        Ok(id)
    }

    pub fn rename_entity(&mut self, id: u64, name: &str) -> Result<(), EngineError> {
        let name = CString::new(name).map_err(EngineError::InvalidName)?;
        // SAFETY: The handle is live and CString provides a valid C string.
        let status = unsafe { vyra_editor_engine_rename_entity(self.raw.as_ptr(), id, name.as_ptr()) };
        self.check_status(status)
    }

    pub fn destroy_entity(&mut self, id: u64) -> Result<(), EngineError> {
        // SAFETY: The bridge validates the opaque ID against the live scene.
        let status = unsafe { vyra_editor_engine_destroy_entity(self.raw.as_ptr(), id) };
        self.check_status(status)
    }

    pub fn clear_scene(&mut self) -> Result<(), EngineError> {
        // SAFETY: The handle is live and no borrowed pointers are passed.
        let status = unsafe { vyra_editor_engine_clear_scene(self.raw.as_ptr()) };
        self.check_status(status)
    }

    fn check_status(&self, status: i32) -> Result<(), EngineError> {
        if status == 0 { Ok(()) } else { Err(self.last_error()) }
    }

    fn last_error(&self) -> EngineError {
        // SAFETY: The bridge error pointer is valid until the next bridge call.
        let message = unsafe { vyra_editor_engine_last_error(self.raw.as_ptr()) };
        if message.is_null() { return EngineError::Native("Unknown native bridge failure".into()); }
        // SAFETY: The bridge returns a NUL-terminated error string.
        EngineError::Native(unsafe { CStr::from_ptr(message) }.to_string_lossy().into_owned())
    }
}

impl Drop for EngineBridge {
    fn drop(&mut self) {
        // SAFETY: This wrapper uniquely owns the handle and drops it exactly once.
        unsafe { vyra_editor_engine_destroy(self.raw.as_ptr()) };
    }
}
