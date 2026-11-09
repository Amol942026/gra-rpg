"""Refine only the new abyss masters, preserving the completed chest artwork."""
import importlib
import krita_abyss_chests


def produce(output, retained):
    art = importlib.reload(krita_abyss_chests)
    output.mkdir(parents=True, exist_ok=False)
    return {"assets": [art.cliff(output, retained), art.panorama(output, retained), art.distance(output, retained)],
            "tool": "Krita native Python / Qt painting", "external_inputs": [], "audio": False}
