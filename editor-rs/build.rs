use std::{env, path::PathBuf};

fn main() {
    println!("cargo:rerun-if-env-changed=VYRA_ENGINE_LIB_DIR");
    if let Ok(directory) = env::var("VYRA_ENGINE_LIB_DIR") {
        let path = PathBuf::from(directory);
        if !path.is_dir() { panic!("VYRA_ENGINE_LIB_DIR is not a directory: {}", path.display()); }
        println!("cargo:rustc-link-search=native={}", path.display());
        println!("cargo:rustc-link-lib=dylib=vyra_editor_bridge");
        #[cfg(any(target_os = "linux", target_os = "macos"))]
        println!("cargo:rustc-link-arg=-Wl,-rpath,{}", path.display());
    } else {
        println!("cargo:warning=VYRA_ENGINE_LIB_DIR is unset; cargo check works, but linking/running requires the native bridge library.");
    }
}
