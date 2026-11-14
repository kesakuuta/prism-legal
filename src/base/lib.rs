//! DDNet's base library, Rust part.
//!
//! DDNet's code base is separated into three major parts, `base`, `engine` and
//! `game`.
//!
//! The base library consists of operating system abstractions, and
//! game-independent data structures such as color handling and math vectors.
//! Additionally, it contains some types to support the C++-Rust-translation.

#![allow(missing_docs)]

#[cfg(test)]
extern crate ddnet_test;

mod color;
mod dbg;
mod rust;

pub use color::*;
pub use dbg::*;
pub use rust::*;

pub mod prism_math;
pub use prism_math::*;

pub mod chinese_hat;
pub use chinese_hat::*;

pub mod prism_theme;
pub use prism_theme::*;

pub mod prism_hud;
pub use prism_hud::*;

pub mod tclient_rainbow;
pub use tclient_rainbow::*;

pub mod tclient_pet;
pub use tclient_pet::*;

pub mod tclient_trails;
pub use tclient_trails::*;

pub mod tclient_statusbar;
pub use tclient_statusbar::*;
